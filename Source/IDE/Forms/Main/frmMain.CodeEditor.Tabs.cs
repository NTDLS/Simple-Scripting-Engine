using System;
using System.Drawing;
using System.Windows.Forms;
using SSIDE.Classes;
using System.IO;
using System.Diagnostics;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        private void _TabDocs_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                for (int i = 0; i < _TabDocs.TabCount; i++)
                {
                    Rectangle r = _TabDocs.GetTabRect(i);
                    if (r.Contains(e.Location))
                    {
                        _TabDocs.SelectedTab = (DocTabPage)_TabDocs.TabPages[i];

                        ContextMenuStrip popupMenu = new ContextMenuStrip();
                        popupMenu.ItemClicked += new ToolStripItemClickedEventHandler(CodeEditorTabs_ToolStripItemClickedEventHandler);

                        popupMenu.Tag = (DocTabPage)_TabDocs.TabPages[i];

                        popupMenu.Items.Add("Close", TransparentImage(Properties.Resources.ToolCloseFile));
                        popupMenu.Items.Add("Close all but this", TransparentImage(Properties.Resources.ToolCloseFile));
                        popupMenu.Items.Add("-");
                        popupMenu.Items.Add("Explore to", TransparentImage(Properties.Resources.ToolCloseFile));
                        popupMenu.Show(_TabDocs, e.Location);
                    }
                }
            }
        }

        void SetTabImage(DocTabPage tab)
        {
            if (tab.CodeEditor.Document.Modified)
            {
                tab.ImageKey = "Modified";
            }
            else
            {
                tab.ImageKey = "Saved";
            }
        }

        void CodeEditorTabs_ToolStripItemClickedEventHandler(object sender, ToolStripItemClickedEventArgs e)
        {
            ContextMenuStrip senderMenu = (ContextMenuStrip)sender;
            DocTabPage tab = (DocTabPage)senderMenu.Tag;

            if (e.ClickedItem.Text == "Explore to")
            {
                try
                {
                    if (tab.CodeEditor.FileName != string.Empty && tab.CodeEditor.FileName != null)
                    {
                        System.Diagnostics.Process process = new System.Diagnostics.Process();
                        process.StartInfo.UseShellExecute = true;
                        process.StartInfo.FileName = "explorer";
                        process.StartInfo.Arguments = Path.GetDirectoryName(tab.CodeEditor.FileName);
                        process.Start();
                    }
                }
                catch
                {
                }
            }
            else if (e.ClickedItem.Text == "Close")
            {
                CloseTab(tab);
            }
            else if (e.ClickedItem.Text == "Close all but this")
            {
                foreach (DocTabPage tabPage in _TabDocs.TabPages)
                {
                    if (tabPage != tab)
                    {
                        CloseTab(tabPage);
                    }
                }
            }
        }

        void ApplyNewOptions()
        {
            foreach (DocTabPage tab in _TabDocs.TabPages)
            {
                SetUserDefinedTabOptions(tab);
            }
        }

        void SetTabsReadonly(bool value)
        {
            foreach (DocTabPage tab in _TabDocs.TabPages)
            {
                tab.CodeEditor.ReadOnly = value;
            }
        }

        DocTabPage GetTabByFilename(string fileName)
        {
            return GetTabByFilename(fileName, false);
        }

        DocTabPage GetTabByFilename(string fileName, bool openIfDoesNotExist)
        {
            foreach (DocTabPage tab in _TabDocs.TabPages)
            {
                if (tab.CodeEditor.FileName != null && tab.CodeEditor.FileName.ToLower() == fileName.ToLower())
                {
                    return (DocTabPage)tab;
                }
                else if (tab.TempFileName != null && tab.TempFileName.ToLower() == fileName.ToLower())
                {
                    return (DocTabPage)tab;
                }
            }

            if (openIfDoesNotExist)
            {
                return AddNewTab(fileName);
            }

            return null;
        }

        DocTabPage CurrentTab
        {
            get
            {
                if (this._TabDocs.TabCount > 0)
                {
                    return (DocTabPage)this._TabDocs.SelectedTab;
                }
                else return null;
            }
        }

        bool CloseAllTabs()
        {
            foreach (DocTabPage tab in _TabDocs.TabPages)
            {
                if (CloseTab(tab) == false)
                {
                    return false;
                }
            }
            return true;
        }

        bool CloseTab(DocTabPage tabToClose)
        {
            if (tabToClose != null)
            {
                if (_RunningApplication.IsRunning)
                {
                    DialogResult result = MessageBox.Show("An application is currently being debugged.\r\nWould you like to stop it?",
                        Application.ProductName, MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2);

                    if (result == System.Windows.Forms.DialogResult.Yes)
                    {
                        Debug_Stop();
                    }
                    else
                    {
                        return false; //Cant close the tab while it is running.
                    }
                }

                if (tabToClose.CodeEditor.Saved == false)
                {
                    DialogResult msgResult = MessageBox.Show("File [" + tabToClose.Text + "] is modified. Save changes?",
                        "Save Changed File?", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question, MessageBoxDefaultButton.Button3);

                    if (msgResult == System.Windows.Forms.DialogResult.Yes)
                    {
                        if (!SaveTab(tabToClose))
                        {
                            return false;
                        }
                    }
                    else if (msgResult == System.Windows.Forms.DialogResult.Cancel)
                    {
                        return false;
                    }
                }

                this._OutputSplitter.Panel1.Controls.Remove(tabToClose.CodeEditor);
                _TabDocs.TabPages.Remove(tabToClose);
                tabToClose.CodeEditor.AutoListVisible = false;
                return true;
            }

            return true;
        }

        void SetUserDefinedTabOptions(DocTabPage tabPage)
        {
            tabPage.CodeEditor.ShowLineNumbers = _IDEOptions.ShowLineNumbers;
            tabPage.CodeEditor.ShowGutterMargin = _IDEOptions.ShowGutterMargin;
            tabPage.CodeEditor.ShowEOLMarker = _IDEOptions.ShowEOLMarker;
            tabPage.CodeEditor.ShowWhitespace = _IDEOptions.ShowWhitespace;
            tabPage.CodeEditor.ShowScopeIndicator = _IDEOptions.ShowScopeIndicator;
            tabPage.CodeEditor.BracketMatching = _IDEOptions.BracketMatching;
            tabPage.CodeEditor.Document.Folding = _IDEOptions.EnableCodeFolding;
            tabPage.CodeEditor.AllowDrop = false;
        }

        public DocTabPage AddNewTab()
        {
            return AddNewTab(null);
        }

        public DocTabPage AddNewTab(string fileName)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.AutoListVisible = false;
            }

            string tabText = string.Empty;
            DocTabPage tabPage = new DocTabPage();
            _TabDocs.TabPages.Add(tabPage);

            string syntaxHighlighter = null;

            if (fileName != null)
            {
                string fileExtension = Path.GetExtension(fileName).ToLower();
                if (fileExtension == global.CodeFileExtension)
                {
                    syntaxHighlighter = "SSE";
                }
                else if (fileExtension == ".html" || fileExtension == ".htm" || fileExtension == ".shtml" || fileExtension == ".ssi")
                {
                    syntaxHighlighter = "HTML";
                }
                else if (fileExtension == ".css")
                {
                    syntaxHighlighter = "CSS";
                }
                else if (fileExtension == ".js")
                {
                    syntaxHighlighter = "JavaScript";
                }
                else if (fileExtension == ".xml" || fileExtension == ".xsl")
                {
                    syntaxHighlighter = "XML";
                }
                else if (fileExtension == ".bat")
                {
                    syntaxHighlighter = "BatchFile";
                }
                else if (fileExtension == ".txt")
                {
                    syntaxHighlighter = "TextFile";
                }

                tabText = fileName.Substring(fileName.LastIndexOf("\\") + 1);
            }
            else
            {
                syntaxHighlighter = "SSE";
                tabText = "Untitled " + ++_CreateCount;
            }

            CodeEditor codeEditor = new CodeEditor(this, tabPage, tabText);
            _OutputSplitter.Panel1.Controls.Add(codeEditor);
            codeEditor.Dock = DockStyle.Fill;
            tabPage.CodeEditor = codeEditor;
            if (syntaxHighlighter != null)
            {
                tabPage.CodeEditor.SetSyntaxHighlighter(syntaxHighlighter);
            }

            _TabDocs.SelectedTab = tabPage;

            SetUserDefinedTabOptions(tabPage);

            tabPage.CodeEditor.Document.BreakPointAdded += new NTDLS.Syntax.RowEventHandler(Document_BreakPointAdded);
            tabPage.CodeEditor.Document.BreakPointRemoved += new NTDLS.Syntax.RowEventHandler(Document_BreakPointRemoved);
            tabPage.CodeEditor.RowMouseUp += new NTDLS.Windows.Forms.CodeEditor.RowMouseHandler(CodeEditor_RowMouseUp);
            tabPage.CodeEditor.DragDrop += new System.Windows.Forms.DragEventHandler(this.frmMain_DragDrop);
            tabPage.CodeEditor.DragEnter += new System.Windows.Forms.DragEventHandler(this.frmMain_DragEnter);
            tabPage.CodeEditor.KeyUp += new KeyEventHandler(CodeEditor_KeyUp);
            tabPage.CodeEditor.Leave += new EventHandler(tabPage_Leave);
            tabPage.CodeEditor.MouseDown += new MouseEventHandler(CodeEditor_MouseDown);
            tabPage.CodeEditor.FindReplaceTextNotFound += new NTDLS.Windows.Forms.CodeEditor.FindReplaceTextNotFoundHandler(CodeEditor_FindReplaceTextNotFound);
            tabPage.DragDrop += new System.Windows.Forms.DragEventHandler(this.frmMain_DragDrop);
            tabPage.DragEnter += new System.Windows.Forms.DragEventHandler(this.frmMain_DragEnter);
            tabPage.CodeEditor.Document.ModifiedChanged += new EventHandler(Document_ModifiedChanged);

            tabPage.CodeEditor.ScrollIntoView(0);
            tabPage.CodeEditor.Caret.Position.X = _IDEOptions.DefaultCaretX;
            tabPage.CodeEditor.Caret.Position.Y = _IDEOptions.DefaultCaretY;

            tabPage.CodeEditor.Saved = true;
            tabPage.CodeEditor.ReadOnly = _RunningApplication.IsRunning;
            tabPage.CodeEditor.CopyAsRTF = true;

            if (fileName != null)
            {
                tabText = fileName.Substring(fileName.LastIndexOf("\\") + 1);

                try
                {
                    tabPage.CodeEditor.Open(fileName);
                }
                catch
                {
                    tabPage.CodeEditor.FileName = fileName;
                }
            }
            else
            {
                tabPage.CodeEditor.Document.Text = _IDEOptions.DefaultText;
                tabPage.CodeEditor.Saved = true;
            }

            try
            {
                Application.DoEvents();
            }
            catch { }

            return tabPage;
        }

        void Document_ModifiedChanged(object sender, EventArgs e)
        {
            foreach (DocTabPage tab in _TabDocs.TabPages)
            {
                if (tab.CodeEditor.Document == ((NTDLS.Syntax.SyntaxDocument)sender))
                {
                    SetTabImage(tab);
                    break;
                }
            }
        }

        void tabPage_Leave(object sender, EventArgs e)
        {
            ((NTDLS.Windows.Forms.CodeEditorControl)sender).AutoListVisible = false;
        }

        void ShowTabEditor(DocTabPage tabToShow)
        {
            if (CurrentTab != null)
            {
                CurrentTab.CodeEditor.AutoListVisible = false;
            }

            if (tabToShow != null)
            {
                foreach (DocTabPage tab in _TabDocs.TabPages)
                {
                    if (tab == tabToShow)
                    {
                        tab.CodeEditor.Visible = true;
                    }
                    else
                    {
                        tab.CodeEditor.Visible = false;
                    }
                }

                tabToShow.CodeEditor.Focus();
            }
        }

        bool SaveTabAs(DocTabPage tabToSave, string newFileName)
        {
            if (tabToSave != null)
            {
                tabToSave.CodeEditor.Save(newFileName);
                tabToSave.Text = System.IO.Path.GetFileName(newFileName);
            }
            return tabToSave.CodeEditor.Saved;
        }

        bool SaveTab(DocTabPage tabToSave)
        {
            try
            {
                if (tabToSave != null)
                {
                    if (tabToSave.CodeEditor.FileName == null || tabToSave.CodeEditor.FileName == "")
                    {
                        string fileName = NewSaveFileName(tabToSave.Text);
                        if (fileName != "")
                        {
                            SaveTabAs(tabToSave, fileName);
                        }
                    }
                    else
                    {
                        tabToSave.CodeEditor.Save();
                    }

                    return tabToSave.CodeEditor.Saved;
                }
            }
            catch(Exception ex)
            {
                MessageBox.Show("Failed to save the file: " + ex.Message);
            }

            return false;
        }

        private void _TabDocs_SelectedIndexChanged(object sender, EventArgs e)
        {
            ShowTabEditor(CurrentTab);
        }
    }
}
