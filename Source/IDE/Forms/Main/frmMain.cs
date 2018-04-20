using System;
using System.Diagnostics;
using System.Drawing;
using System.IO.Pipes;
using System.Linq;
using System.Windows.Forms;
using SSIDE.Classes;
using System.IO;

namespace SSIDE.Forms
{
    public partial class frmMain : Form
    {
        #region Types: Public

        public struct RunningApplication
        {
            public string InstanceId;
            public Process Process;
            public bool IsRunning;
            public bool WasAttach;
            public int AttachProcessId;
            public System.Threading.Thread ProcessMonitorThread;

            public System.Threading.Thread NamedPipeThreadTxt;
            public System.Threading.Thread NamedPipeThreadCmd;
            public System.Threading.Thread NamedPipeThreadErr;

            public NamedPipeClientStream ReadCmdPipe;
            public NamedPipeClientStream ReadErrPipe;
            public NamedPipeClientStream ReadTxtPipe;
            public NamedPipeClientStream WriteCmdPipe;

            public String Engine;
            public String Script;
            public DocTabPage OriginalTab;
            public frmMain Form;

            public deAddErrorToList pAddErrorToList;
            public deAddOutputToList pAddOutputToList;
            public deExecutionBegin pExecutionBegin;
            public deExecutionComplete pExecutionComplete;
            public deBreakPointHit pBreakPointHit;
            public deUpdateWatchValue pUpdateWatchValue;
            public deAddImmediateInfo pAddImmediateInfo;
            public deUpdateLocalsValue pUpdateLocalsValue;
            public deAddFileToGrid pAddFileToGrid;
            public deAutosizeFileGrid pAutosizeFileGrid;
            public deClearFilesGrid pClearFilesGrid;
            public deRemoveNonUpdatedLocals pRemoveNonUpdatedLocals;
            public deToolTipSymbolInfo pToolTipSymbolInfo;
            public deQuickWatchInfo pQuickWatchInfo;
            public deImmediateAutoListBegin pImmediateAutoListBegin;
            public deImmediateAutoListEnd pImmediateAutoListEnd;
            public deImmediateAutoListAddWord pImmediateAutoListAddWord;

            public NTDLS.Syntax.Row ExecuteLine;
            public ToolStripMenuItem TipMenuItem;
        }

        #endregion

        #region Variables: Private

        private int _CreateCount = 0;
        private RunningApplication _RunningApplication;
        private IDEOptions _IDEOptions = new IDEOptions();
        private frmWatchExpression _WatchForm = new frmWatchExpression();
        HtmlHelpViewer.Viewer _HelpViewer = null;
        //HtmlHelpViewer.Viewer _ContextHelp = null;

        #endregion

        #region Constructor(s) / Deconstructor

        public frmMain()
        {
            InitializeComponent();
        }

        #endregion

        #region Properties

        public bool ShowProjectSection
        {
            get{return !_ProjectSplitter.Panel1Collapsed;}
            set{_ProjectSplitter.Panel1Collapsed = !value;}
        }

        public bool ShowToolsSection
        {
            get{return !_ToolsSplitter.Panel2Collapsed;}
            set{_ToolsSplitter.Panel2Collapsed = !value;}
        }

        public bool ShowOutputSection
        {
            get{return !_OutputSplitter.Panel2Collapsed;}
            set{_OutputSplitter.Panel2Collapsed = !value;}
        }
        
        #endregion

        #region Form: Events

        private void frmMain_DragDrop(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
            if (files != null)
            {
                foreach (string fullFileAndPath in files)
                {
                    if (Path.GetExtension(fullFileAndPath).ToLower() == global.ProjectFileExtension)
                    {
                        LoadProjectFile(fullFileAndPath);
                    }
                    else
                    {
                        AddNewTab(fullFileAndPath);
                    }
                }
            }
        }

        private void frmMain_DragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop, false) == true)
            {
                e.Effect = DragDropEffects.All;
            }
        }

        private void frmMain_Load(object sender, EventArgs e)
        {
            InitializeErrorsGrid();
            InitializeWatchGrid();
            InitializeLocalsGrid();
            InitializeFilesGrid();

            _OutputSplitter.AllowDrop = true;
            _OutputSplitter.Panel1.AllowDrop = true;
            _OutputSplitter.Panel1.DragEnter += frmMain_DragEnter;
            _OutputSplitter.Panel1.DragDrop += frmMain_DragDrop;

            _OutputSplitter.Panel2.AllowDrop = true;
            _OutputSplitter.Panel2.DragEnter += frmMain_DragEnter;
            _OutputSplitter.Panel2.DragDrop += frmMain_DragDrop;

            ShowProjectSection = true;
            ShowToolsSection = false;
            ShowOutputSection = true;

            if (!IsProjectOpen())
            {
                AddDefaultProjectNode();
            }

            //Initialize misc.
            if (_TabDocs.TabPages.Count == 0 && !IsProjectOpen())
            {
                AddNewTab();
            }

            SetIconsStopped();

            //Screen currentScreen = Screen.FromPoint(this.Location);
            //this.Height = (int) (currentScreen.WorkingArea.Height * 0.95);
            //this.Width = (int) (currentScreen.WorkingArea.Width * 0.95);
            //this.CenterToScreen();
        }

        private void frmMain_Shown(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Focus();
            }
        }

        private void frmMain_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (_RunningApplication.IsRunning)
            {
                DialogResult result = MessageBox.Show("An application is currently being debugged.\r\nWould you like to stop it?",
                    Application.ProductName, MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2);

                if (result == System.Windows.Forms.DialogResult.Yes)
                {
                    Debug_Stop();
                }
                else if (result == System.Windows.Forms.DialogResult.No)
                {
                    e.Cancel = true;
                    return;
                }
            }

            if (CloseAllTabs() == false)
            {
                e.Cancel = true;
            }
        }

        #endregion

        #region File: Open / Save

        string NewOpenFileName()
        {
            System.Windows.Forms.OpenFileDialog ofdScript = new System.Windows.Forms.OpenFileDialog();

            ofdScript.Multiselect = false;
            ofdScript.ShowHelp = false;
            ofdScript.DefaultExt = global.CodeFileExtension;
            ofdScript.CheckFileExists = true;
            ofdScript.CheckPathExists = true;
            ofdScript.ValidateNames = true;
            ofdScript.Filter = "Simple Scripting & Projects|*" + global.CodeFileExtension + ";*" + global.ProjectFileExtension + "|Text Files|*.txt|All Files|*.*";

            if (ofdScript.ShowDialog() == DialogResult.OK)
            {
                return ofdScript.FileName;
            }

            return string.Empty;
        }

        string NewSaveFileName()
        {
            return NewSaveFileName("New Script" + global.CodeFileExtension);
        }

        string NewSaveFileName(string defaultFileName)
        {
            System.Windows.Forms.SaveFileDialog ofdScript = new System.Windows.Forms.SaveFileDialog();

            ofdScript.ShowHelp = false;
            ofdScript.DefaultExt = global.CodeFileExtension;
            ofdScript.CheckPathExists = true;
            ofdScript.ValidateNames = true;
            ofdScript.Filter = "Simple Script|*" + global.CodeFileExtension + "|Text Files|*.txt|All Files|*.*";
            ofdScript.FileName = defaultFileName;

            if (ofdScript.ShowDialog() == DialogResult.OK)
            {
                return ofdScript.FileName;
            }

            return string.Empty;
        }

        #endregion

        #region Menu: Click Events

        private void attachToolStripMenuItem_DropDownOpening(object sender, EventArgs e)
        {
            System.Windows.Forms.ToolStripDropDownItem parentMenuItem = ((System.Windows.Forms.ToolStripDropDownItem)(sender));

            parentMenuItem.DropDownItems.Clear();

            String[] listOfPipes = System.IO.Directory.GetFiles(@"\\.\pipe\");

            var pipeNames = (
                from n in listOfPipes
                where n.StartsWith("\\\\.\\pipe\\SSE_")
                    && n.Contains("_IDE_") == false
                select n.Substring(18)).Distinct().ToList();

            int addedCount = 0;

            if (pipeNames.Count() > 0)
            {
                foreach (string pipeName in pipeNames)
                {
                    int processId = 0;

                    if (int.TryParse(pipeName, out processId))
                    {
                        Process sseProcess = Process.GetProcessById(processId);

                        string userName = "";
                        string userDomain = "";

                        userDomain = sseProcess.StartInfo.EnvironmentVariables["userdomain"];
                        userName = sseProcess.StartInfo.EnvironmentVariables["username"];
                        if (userDomain != "")
                        {
                            userDomain += "\\";
                        }

                        //foreach (System.Collections.DictionaryEntry ss in sseProcess.StartInfo.EnvironmentVariables)
                        //{
                        //    MessageBox.Show(ss.Key + " = " + ss.Value);
                        //}

                        string caption = sseProcess.ProcessName + " (PID:" + processId + ", User:" + userDomain + userName + ")";
                        ToolStripItem menuItem = parentMenuItem.DropDownItems.Add(caption);
                        menuItem.Click += new EventHandler(Debug_Attach_MenuItem_Click);
                        menuItem.Tag = processId;
                        menuItem.Enabled = !_RunningApplication.IsRunning;

                        addedCount++;
                    }
                }
            }

            if (addedCount == 0)
            {
                parentMenuItem.DropDownItems.Add("(no processes found)").Enabled = false;
            }
        }

        void Debug_Attach_MenuItem_Click(object sender, EventArgs e)
        {
            ToolStripItem menuItem = (ToolStripItem)sender;
            Debug_Attach((int)menuItem.Tag);
        }

        private void outputToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowOutputSection = !ShowOutputSection;
        }

        private void projectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowProjectSection = !ShowProjectSection;
        }

        private void toolsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ShowToolsSection = !ShowToolsSection;
        }

        private void toolbarToolStripMenuItem_Click(object sender, EventArgs e)
        {
            toolStrip.Visible = !toolStrip.Visible;
            toolbarToolStripMenuItem.Checked = toolStrip.Visible;
        }

        private void optionsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            frmOptions optionsForm = new frmOptions(_IDEOptions);
            if (optionsForm.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                ApplyNewOptions();
            }
        }

        private void exportToolStripMenuItem_Click(object sender, EventArgs e)
        {
            frmExport exportForm = new frmExport(CurrentTab);
            exportForm.ShowDialog();
        }

        private void snippetsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                string filePath = global.GetRegistryString("", "Path");
                frmCodeBrowser form = new frmCodeBrowser(filePath + "\\IDE\\Snippets", true, "Snippets",
                    "Snippets provide an easy method of saving frequently used peices of code and inserting them into your project.");
                if (form.ShowDialog() == DialogResult.OK)
                {
                    CurrentTab.CodeEditor.Document.InsertText(
                        form.CodeText,
                        CurrentTab.CodeEditor.Caret.Position.X,
                        CurrentTab.CodeEditor.Caret.Position.Y, true);
                }
            }
            else
            {
                MessageBox.Show("No tab is currently active.");
            }
        }

        private void examplesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string filePath = global.GetRegistryString("", "Path");
            frmCodeBrowser form = new frmCodeBrowser(filePath + "\\IDE\\Examples", false, "Examples",
                "These are functionality examples and can be used to learn, study and save large portions of code.");
            if (form.ShowDialog() == DialogResult.OK)
            {
                DocTabPage tabPage = AddNewTab();
                tabPage.CodeEditor.Document.Text = form.CodeText;
            }
        }

        private void libraryToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string filePath = global.GetRegistryString("", "Path");
            frmCodeBrowser form = new frmCodeBrowser(filePath + "\\Library", false, "Standard Library",
                "This is the Simple Scriping Engine standard abstraction library. These scripts can be included into your projects "
                + " to provide a seamless interface between the standard windows API and the scripting engine.");
            if (form.ShowDialog() == DialogResult.OK)
            {
                DocTabPage tabPage = AddNewTab(form.Filename);
            }
        }

        private void About_Menu_Click(object sender, EventArgs e)
        {
            frmAbout aboutForm = new frmAbout();
            aboutForm.ShowDialog();
        }

        private void checkForUpdateToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string sDocumentation = "\"" + global.GetRegistryString("", "Path") + "\\AutoUpdate.Exe\"";

            try
            {
                ProcessStartInfo processStartInfo = new ProcessStartInfo(sDocumentation);
                processStartInfo.UseShellExecute = true;
                processStartInfo.WindowStyle = ProcessWindowStyle.Normal;
                Process.Start(processStartInfo);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void Contents_Menu_Click(object sender, EventArgs e)
        {
            string helpPath = global.GetRegistryString("", "Path", null);
            if (helpPath != null)
            {
                string sDocumentation = helpPath + "\\Help\\Help.chm";

                if (_HelpViewer == null || _HelpViewer.Visible == false)
                {
                    _HelpViewer = new HtmlHelpViewer.Viewer();

                    _HelpViewer.Show();
                    if (!_HelpViewer.OpenFile(sDocumentation))
                    {
                        _HelpViewer.Close();
                    }
                }
                else
                {
                    _HelpViewer.Focus();
                }
                if (CurrentTab != null)
                {
                    NTDLS.Windows.Forms.CodeEditor.Selection selection = CurrentTab.CodeEditor.Selection;
                    if (selection != null && selection.Text.Length > 0)
                    {
                        //_HelpViewer.IndexSelect(selection.Text.Trim());
                        _HelpViewer.SearchForText(selection.Text.Trim().Replace(".", " "));
                    }
                }
            }
        }

        private void New_Project_Menu_Click(object sender, EventArgs e)
        {
            System.Windows.Forms.SaveFileDialog ofdScript = new System.Windows.Forms.SaveFileDialog();

            ofdScript.ShowHelp = false;
            ofdScript.DefaultExt = global.ProjectFileExtension;
            ofdScript.CheckPathExists = true;
            ofdScript.ValidateNames = true;
            ofdScript.Filter = "Simple Scripting Project|*" + global.ProjectFileExtension;
            ofdScript.FileName = "New project" + global.ProjectFileExtension;

            if (ofdScript.ShowDialog() == DialogResult.OK)
            {
                string projectPath = (Path.GetDirectoryName(ofdScript.FileName) + "\\" + Path.GetFileNameWithoutExtension(ofdScript.FileName)).Replace("\\\\", "\\");
                string projectFile = (projectPath + "\\" + Path.GetFileName(ofdScript.FileName)).Replace("\\\\", "\\");
                Directory.CreateDirectory(projectPath);

                if (CreateEmptyProject(projectFile))
                {
                }
            }
        }

        private void New_Menu_Click(object sender, EventArgs e)
        {
            AddNewTab();
        }

        private void Open_Menu_Click(object sender, EventArgs e)
        {
            string fileName = NewOpenFileName();
            if (fileName != "")
            {
                if (Path.GetExtension(fileName).ToLower() == global.ProjectFileExtension)
                {
                    LoadProjectFile(fileName);
                }
                else
                {
                    //What is the difference between these two methods?
                    //DocTabPage tabPage = AddNewTab();
                    //tabPage.CodeEditor.Open(fileName);
                    AddNewTab(fileName);
                }
            }
        }

        private void closeCurrentProjectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CloseProject();
        }

        private void Close_Menu_Click(object sender, EventArgs e)
        {
            CloseTab(CurrentTab);
        }

        private void Save_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                SaveTab(CurrentTab);
            }
        }

        private void SaveAs_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                string fileName = NewSaveFileName(CurrentTab.Text);
                if (fileName != "")
                {
                    SaveTabAs(CurrentTab, fileName);
                }
            }
        }

        private void SaveAll_Menu_Click(object sender, EventArgs e)
        {
            SaveProjectFile();
            foreach (DocTabPage tab in _TabDocs.TabPages)
            {
                SaveTab((DocTabPage)tab);
            }
        }

        private void Exit_Menu_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void Undo_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Undo();
            }
        }

        private void Redo_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Redo();
            }
        }

        private void Cut_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Cut();
            }
        }

        private void Copy_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Copy();
            }
        }

        private void Paste_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Paste();
            }
        }

        private void Delete_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Delete();
            }
        }

        private void SelectAll_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.SelectAll();
            }
        }

        private void Find_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                string findText = "";

                NTDLS.Windows.Forms.CodeEditor.Selection selection = CurrentTab.CodeEditor.Selection;
                if (selection != null && selection.Text.Length > 0)
                {
                    findText = selection.Text;
                }
                else
                {
                    int caretX = CurrentTab.CodeEditor.Caret.Position.X;
                    int caretY = CurrentTab.CodeEditor.Caret.Position.Y;
                    if (caretX >= 0 && caretY >= 0)
                    {
                        NTDLS.Syntax.TextPoint currentWordPos = new NTDLS.Syntax.TextPoint(caretX, caretY);
                        NTDLS.Syntax.Word currentWord = CurrentTab.CodeEditor.Document.GetValidWordFromPos(currentWordPos);
                        if (currentWord != null && currentWord.Text.Length > 0)
                        {
                            findText = currentWord.Text.Trim();
                            if (findText.Substring(findText.Length - 1) == ",")
                            {
                                findText = findText.Substring(0, findText.Length - 1);
                            }
                        }
                    }
                }

                CurrentTab.CodeEditor.ShowFind(findText);
            }
        }

        private void FindNext_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                if (CurrentTab.CodeEditor.LastSearchString() == "")
                {
                    Find_Menu_Click(sender, e);
                }
                else
                {
                    CurrentTab.CodeEditor.FindNext();
                }
            }
        }

        private void Replace_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                string findText = "";

                NTDLS.Windows.Forms.CodeEditor.Selection selection = CurrentTab.CodeEditor.Selection;
                if (selection != null && selection.Text.Length > 0)
                {
                    findText = selection.Text;
                }
                else
                {
                    int caretX = CurrentTab.CodeEditor.Caret.Position.X;
                    int caretY = CurrentTab.CodeEditor.Caret.Position.Y;
                    if (caretX >= 0 && caretY >= 0)
                    {
                        NTDLS.Syntax.TextPoint currentWordPos = new NTDLS.Syntax.TextPoint(caretX, caretY);
                        NTDLS.Syntax.Word currentWord = CurrentTab.CodeEditor.Document.GetValidWordFromPos(currentWordPos);
                        if (currentWord != null && currentWord.Text.Length > 0)
                        {
                            findText = currentWord.Text.Trim();
                            if (findText.Substring(findText.Length - 1) == ",")
                            {
                                findText = findText.Substring(0, findText.Length - 1);
                            }
                        }
                    }
                }

                CurrentTab.CodeEditor.ShowReplace(findText);
            }
        }

        private void GoToLine_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.ShowGotoLine();
            }
        }

        private void ClearAllBookmarks_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                DialogResult result = MessageBox.Show("Clear all code bookmarks?",
                    Application.ProductName, MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2);

                if (result == System.Windows.Forms.DialogResult.Yes)
                {
                    CurrentTab.CodeEditor.Document.ClearBookmarks();
                }
            }
        }

        private void Run_Menu_Click(object sender, EventArgs e)
        {
            ProjectTreeNode projectNode = FindDefaultRunProjectFile();
            if (projectNode != null)
            {
                string defaultRunFileName = projectNode.LogicalPath();

                DocTabPage defaultTab = GetTabByFilename(defaultRunFileName);
                if (defaultTab == null)
                {
                    defaultTab = AddNewTab(defaultRunFileName);
                }

                if (defaultTab != null)
                {
                    defaultTab.CodeEditor.Focus();
                    _TabDocs.SelectedTab = defaultTab;
                }
            }

            Debug_Start(CurrentTab);
        }

        private void Pause_Menu_Click(object sender, EventArgs e)
        {
            Debug_Break_Pause();
        }

        private void Stop_Menu_Click(object sender, EventArgs e)
        {
            Debug_Stop();
        }

        private void StepInto_Menu_Click(object sender, EventArgs e)
        {
            Debug_Break_StepInto();
        }

        private void StepOver_Menu_Click(object sender, EventArgs e)
        {
            Debug_Break_StepOver();
        }

        private void StepOut_Menu_Click(object sender, EventArgs e)
        {
            Debug_Break_StepOut();
        }

        private void ClearAllBreakPoints_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                DialogResult result = MessageBox.Show("Clear all code breakpoints?",
                    Application.ProductName, MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2);

                if (result == System.Windows.Forms.DialogResult.Yes)
                {
                    CurrentTab.CodeEditor.Document.ClearBreakpoints();
                }
            }
        }

        private void ToggleBreakpoint_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Caret.CurrentRow.Breakpoint = !CurrentTab.CodeEditor.Caret.CurrentRow.Breakpoint;
            }
        }

        private void IncreaseIndent_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Selection.Indent();
            }
        }

        private void DecreaseIndent_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Selection.Outdent();
            }
        }

        private void CommentSelection_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Selection.Indent(";");
            }
        }

        private void UncommentSelection_Menu_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.Selection.Outdent(";");
            }
        }

        #endregion

        #region ToolBar: Utility

        Image TransparentImage(Image image)
        {
            Bitmap toolBitmap = new Bitmap(image);
            toolBitmap.MakeTransparent(Color.Magenta);
            return toolBitmap;
        }

        void SyncMenuWithToolBar()
        {
            Stop_Menu.Enabled = cmdStop.Enabled;
            Pause_Menu.Enabled = cmdPause.Enabled;
            StepInto_Menu.Enabled = cmdStepInto.Enabled;
            StepOut_Menu.Enabled = cmdStepOut.Enabled;
            StepOver_Menu.Enabled = cmdStepOver.Enabled;
            Run_Menu.Enabled = cmdRun.Enabled;
        }

        void SetIconsRunning()
        {
            SetTabsReadonly(true); //This is set back to writeable in [SetIconsStopped()].

            cmdStop.Enabled = true;
            cmdPause.Enabled = true;
            cmdStepInto.Enabled = false;
            cmdStepOut.Enabled = false;
            cmdStepOver.Enabled = false;
            cmdRun.Enabled = false;

            _ImmediateText.ReadOnly = true;

            _DebugLineText.Text = "";

            SyncMenuWithToolBar();

            if (_RunningApplication.ExecuteLine != null)
            {
                _RunningApplication.ExecuteLine.BackColor = Color.White;
            }
        }

        void SetIconsPaused()
        {
            cmdStop.Enabled = true;
            cmdPause.Enabled = false;
            cmdStepInto.Enabled = true;
            cmdStepOut.Enabled = true;
            cmdStepOver.Enabled = true;
            cmdRun.Enabled = true;

            _ImmediateText.ReadOnly = false;

            //ToggleBreakpoint_Menu.Enabled = false;
            //ClearAllBreakPoints_Menu.Enabled = false;

            SyncMenuWithToolBar();
        }

        void SetIconsStopped()
        {
            cmdStop.Enabled = false;
            cmdPause.Enabled = false;
            cmdStepInto.Enabled = false;
            cmdStepOut.Enabled = false;
            cmdStepOver.Enabled = false;
            cmdRun.Enabled = (_TabDocs.TabPages.Count > 0 || IsProjectOpen());

            _ImmediateText.ReadOnly = true;

            //ToggleBreakpoint_Menu.Enabled = true;
            //ClearAllBreakPoints_Menu.Enabled = true;

            _DebugLineText.Text = "";

            SyncMenuWithToolBar();

            if (_RunningApplication.ExecuteLine != null)
            {
                _RunningApplication.ExecuteLine.BackColor = Color.White;
            }

            SetTabsReadonly(false); //This is set back to wriable in [SetIconsStopped()].
        }

        #endregion

        #region ToolBar: Click Events

        private void cmdNewFile_Click(object sender, EventArgs e)
        {
            New_Menu_Click(null, null);
        }

        private void cmdCloseFile_Click(object sender, EventArgs e)
        {
            Close_Menu_Click(null, null);
        }

        private void cmdOpenFile_Click(object sender, EventArgs e)
        {
            Open_Menu_Click(null, null);
        }

        private void cmdSave_Click(object sender, EventArgs e)
        {
            Save_Menu_Click(null, null);
        }

        private void cmdSaveAll_Click(object sender, EventArgs e)
        {
            SaveAll_Menu_Click(null, null);
        }

        private void cmdCloseFile_Click_1(object sender, EventArgs e)
        {
            Close_Menu_Click(null, null);
        }

        private void cmdCut_Click(object sender, EventArgs e)
        {
            Cut_Menu_Click(null, null);
        }

        private void cmdCopy_Click(object sender, EventArgs e)
        {
            Copy_Menu_Click(null, null);
        }

        private void cmdPaste_Click(object sender, EventArgs e)
        {
            Paste_Menu_Click(null, null);
        }

        private void cmdReplace_Click(object sender, EventArgs e)
        {
            Replace_Menu_Click(null, null);
        }

        private void cmdFind_Click(object sender, EventArgs e)
        {
            Find_Menu_Click(null, null);
        }

        private void cmdUndo_Click(object sender, EventArgs e)
        {
            Undo_Menu_Click(null, null);
        }

        private void cmdRedo_Click(object sender, EventArgs e)
        {
            Redo_Menu_Click(null, null);
        }

        private void cmdDecreaseIndent_Click(object sender, EventArgs e)
        {
            DecreaseIndent_Menu_Click(null, null);
        }

        private void cmdIncreaseIndent_Click(object sender, EventArgs e)
        {
            IncreaseIndent_Menu_Click(null, null);
        }

        private void cmdCommentLines_Click(object sender, EventArgs e)
        {
            CommentSelection_Menu_Click(null, null);
        }

        private void cmdUncommentLines_Click(object sender, EventArgs e)
        {
            UncommentSelection_Menu_Click(null, null);
        }

        private void cmdWebRun_Click(object sender, EventArgs e)
        {
            Run_Menu_Click(null, null);
        }

        private void cmdRun_Click(object sender, EventArgs e)
        {
            Run_Menu_Click(null, null);
        }

        private void cmdPause_Click(object sender, EventArgs e)
        {
            Pause_Menu_Click(null, null);
        }

        private void cmdStop_Click(object sender, EventArgs e)
        {
            Stop_Menu_Click(null, null);
        }

        private void cmdStepOver_Click(object sender, EventArgs e)
        {
            StepOver_Menu_Click(null, null);
        }

        private void cmdStepInto_Click(object sender, EventArgs e)
        {
            StepInto_Menu_Click(null, null);
        }

        private void cmdStepOut_Click(object sender, EventArgs e)
        {
            StepOut_Menu_Click(null, null);
        }

        private void cmdToggleOutput_Click(object sender, EventArgs e)
        {
            outputToolStripMenuItem_Click(null, null);
        }

        private void cmdToggleProjectPanel_Click(object sender, EventArgs e)
        {
            projectToolStripMenuItem_Click(null, null);
        }

        private void cmdToggleToolsPanel_Click(object sender, EventArgs e)
        {
            toolsToolStripMenuItem_Click(null, null);
        }

        private void cmdSnippets_Click(object sender, EventArgs e)
        {
            if (CurrentTab != null)
            {
                string filePath = global.GetRegistryString("", "Path");
                frmCodeBrowser form = new frmCodeBrowser(filePath + "\\IDE\\Snippets", true, "Snippets",
                    "Snippets provide an easy method of saving frequently used peices of code and inserting them into your project.");
                if (form.ShowDialog() == DialogResult.OK)
                {
                    CurrentTab.CodeEditor.Document.InsertText(
                        form.CodeText,
                        CurrentTab.CodeEditor.Caret.Position.X,
                        CurrentTab.CodeEditor.Caret.Position.Y, true);
                }
            }
        }
        #endregion
    }
}
