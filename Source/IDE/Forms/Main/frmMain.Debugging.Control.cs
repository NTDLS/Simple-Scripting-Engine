using System;
using System.Windows.Forms;
using SSIDE.Classes;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        void Debug_Attach(int attachProcessId)
        {
            lock (this)
            {
                if (_RunningApplication.IsRunning)
                {
                    //Code is already running but the run icon was enabled... must be a debug continue.
                    Debug_Break_Continue();
                    return;
                }

                _RunningApplication = new RunningApplication();

                _RunningApplication.OriginalTab = null;
                _RunningApplication.Form = this;
                _RunningApplication.InstanceId = attachProcessId.ToString();
                _RunningApplication.IsRunning = true;
                _RunningApplication.WasAttach = true;
                _RunningApplication.AttachProcessId = attachProcessId;
                _RunningApplication.pAddErrorToList = AddErrorToList;
                _RunningApplication.pAddOutputToList = AddOutputToList;
                _RunningApplication.pExecutionBegin = ExecutionBegin;
                _RunningApplication.pExecutionComplete = ExecutionComplete;
                _RunningApplication.pBreakPointHit = BreakPointHit;
                _RunningApplication.pUpdateWatchValue = UpdateWatchValue;
                _RunningApplication.pQuickWatchInfo = QuickWatchInfo;
                _RunningApplication.pAddImmediateInfo = AddImmediateInfo;
                _RunningApplication.pUpdateLocalsValue = UpdateLocalsValue;
                _RunningApplication.pAddFileToGrid = AddFileToGrid;
                _RunningApplication.pAutosizeFileGrid = AutosizeFileGrid;
                _RunningApplication.pClearFilesGrid = ClearFilesGrid;
                _RunningApplication.pRemoveNonUpdatedLocals = RemoveNonUpdatedLocals;
                _RunningApplication.pToolTipSymbolInfo = ToolTipSymbolInfo;
                _RunningApplication.pImmediateAutoListBegin = ImmediateAutoListBegin;
                _RunningApplication.pImmediateAutoListEnd = ImmediateAutoListEnd;
                _RunningApplication.pImmediateAutoListAddWord = ImmediateAutoListAddWord;

                DeleteAllSystemAddedWatchValues();

                SetIconsRunning();

                _ErrorGrid.Rows.Clear();

                if (_OutputSplitter.Panel2Collapsed)
                {
                    _OutputSplitter.Panel2Collapsed = false;
                }

                _OutputBox.Text = "";

                _RunningApplication.ProcessMonitorThread = new System.Threading.Thread(ProcessMonitorThread);
                _RunningApplication.ProcessMonitorThread.Start();

                //Somthing went wrong if the thread was not started, reset everyting back to a non-running state.
                if (_RunningApplication.ProcessMonitorThread == null)
                {
                    _RunningApplication.IsRunning = false;
                    SetIconsStopped();
                    _RunningApplication.Form.Invoke(_RunningApplication.pExecutionComplete);
                }
            }
        }

        void Debug_Start(DocTabPage currentTab)
        {
            lock (this)
            {
                if (currentTab == null)
                {
                    return;
                }

                if (_RunningApplication.IsRunning)
                {
                    //Code is already running but the run icon was enabled... must be a debug continue.
                    Debug_Break_Continue();
                    return;
                }

                _RunningApplication = new RunningApplication();

                _RunningApplication.OriginalTab = currentTab;
                _RunningApplication.Form = this;
                _RunningApplication.InstanceId = "IDE_" + Guid.NewGuid().ToString();
                _RunningApplication.IsRunning = true;
                _RunningApplication.WasAttach = false;
                _RunningApplication.AttachProcessId = 0;
                _RunningApplication.pAddErrorToList = AddErrorToList;
                _RunningApplication.pAddOutputToList = AddOutputToList;
                _RunningApplication.pExecutionBegin = ExecutionBegin;
                _RunningApplication.pExecutionComplete = ExecutionComplete;
                _RunningApplication.pBreakPointHit = BreakPointHit;
                _RunningApplication.pUpdateWatchValue = UpdateWatchValue;
                _RunningApplication.pQuickWatchInfo = QuickWatchInfo;
                _RunningApplication.pAddImmediateInfo = AddImmediateInfo;
                _RunningApplication.pUpdateLocalsValue = UpdateLocalsValue;
                _RunningApplication.pAddFileToGrid = AddFileToGrid;
                _RunningApplication.pAutosizeFileGrid = AutosizeFileGrid;
                _RunningApplication.pClearFilesGrid = ClearFilesGrid;
                _RunningApplication.pRemoveNonUpdatedLocals = RemoveNonUpdatedLocals;
                _RunningApplication.pToolTipSymbolInfo = ToolTipSymbolInfo;
                _RunningApplication.pImmediateAutoListBegin = ImmediateAutoListBegin;
                _RunningApplication.pImmediateAutoListEnd = ImmediateAutoListEnd;
                _RunningApplication.pImmediateAutoListAddWord = ImmediateAutoListAddWord;

                DeleteAllSystemAddedWatchValues();

                SetIconsRunning();

                if (currentTab != null)
                {
                    _ErrorGrid.Rows.Clear();

                    if (_OutputSplitter.Panel2Collapsed)
                    {
                        _OutputSplitter.Panel2Collapsed = false;
                    }

                    _OutputBox.Text = "";

                    bool isTempFile = false;
                    string scriptFileName = "";

                    if (currentTab.CodeEditor.FileName == "" || currentTab.CodeEditor.FileName == null)
                    {
                        string tabText = currentTab.Text;

                        System.Random rand = new System.Random();
                        Double randomValue = rand.NextDouble() * 1234.0;
                        scriptFileName = System.IO.Path.GetTempPath() + "\\Script_" + randomValue + global.CodeFileExtension;

                        scriptFileName = scriptFileName.Replace("/", "\\");
                        scriptFileName = scriptFileName.Replace("\\\\", "\\");

                        currentTab.CodeEditor.Save(scriptFileName);
                        currentTab.Text = tabText;

                        isTempFile = true;
                    }
                    else
                    {
                        scriptFileName = currentTab.CodeEditor.FileName;
                        if (!SaveTab(currentTab))
                        {
                            _RunningApplication.IsRunning = false;
                            SetIconsStopped();
                            return;
                        }
                    }

                    _RunningApplication.Engine = global.GetRegistryString("", "Engine");
                    _RunningApplication.Script = scriptFileName;
                    if (isTempFile)
                    {
                        currentTab.TempFileName = scriptFileName;
                    }
                    else
                    {
                        currentTab.TempFileName = null;
                    }

                    if (_RunningApplication.Engine != "" && _RunningApplication.Engine != null)
                    {
                        if (System.IO.File.Exists(_RunningApplication.Engine))
                        {
                            _RunningApplication.ProcessMonitorThread = new System.Threading.Thread(ProcessMonitorThread);
                            _RunningApplication.ProcessMonitorThread.Start();
                        }
                        else
                        {
                            MessageBox.Show("Cannot find the scripting engine. Is it installed?");
                        }
                    }
                    else
                    {
                        MessageBox.Show("Cannot find the scripting engine. Is it installed?");
                    }
                }

                //Somthing went wrong if the thread was not started, reset everyting back to a non-running state.
                if (_RunningApplication.ProcessMonitorThread == null)
                {
                    _RunningApplication.IsRunning = false;
                    SetIconsStopped();
                    _RunningApplication.Form.Invoke(_RunningApplication.pExecutionComplete);
                }
            }
        }

        void WaitOnThread(System.Threading.Thread thread)
        {
            if (thread != null)
            {
                while (thread.ThreadState != System.Threading.ThreadState.Stopped && thread.ThreadState != System.Threading.ThreadState.Aborted)
                {
                    Application.DoEvents();
                    System.Threading.Thread.Sleep(100);
                }
            }
        }

        void Debug_Stop()
        {
            if (_RunningApplication.IsRunning)
            {
                if (_RunningApplication.WasAttach == false)
                {
                    _RunningApplication.Process.Kill();
                    _RunningApplication.Process.WaitForExit();
                }
                else
                {
                    _RunningApplication.IsRunning = false; //Detach.
                }

                try
                {
                    _RunningApplication.WriteCmdPipe.Dispose();
                    _RunningApplication.ReadTxtPipe.Dispose();
                    _RunningApplication.ReadErrPipe.Dispose();
                    _RunningApplication.ReadErrPipe.Dispose();
                }
                catch
                {
                }

                WaitOnThread(_RunningApplication.NamedPipeThreadTxt);
                WaitOnThread(_RunningApplication.NamedPipeThreadCmd);
                WaitOnThread(_RunningApplication.NamedPipeThreadErr);
                WaitOnThread(_RunningApplication.ProcessMonitorThread);

                SetIconsStopped(); //This is not actually necessary... but it makes me feel better!
            }
        }

        void Debug_Break_Continue()
        {
            SetIconsRunning();
            ((DocTabPage)_TabDocs.SelectedTab).CodeEditor.Focus();
            WriteToCmdPipe("::Continue~");
        }

        void Debug_Break_StepOver()
        {
            SetIconsRunning();
            ((DocTabPage)_TabDocs.SelectedTab).CodeEditor.Focus();
            WriteToCmdPipe("::StepOver~");
        }

        void Debug_Break_StepInto()
        {
            SetIconsRunning();
            ((DocTabPage)_TabDocs.SelectedTab).CodeEditor.Focus();
            WriteToCmdPipe("::StepInto~");
        }

        void Debug_Break_StepOut()
        {
            SetIconsRunning();
            ((DocTabPage)_TabDocs.SelectedTab).CodeEditor.Focus();
            WriteToCmdPipe("::StepOut~");
        }

        void Debug_Break_Pause()
        {
            if (_TabDocs.SelectedTab != null)
            {
                //SetIconsPaused(); This will be handled when the application actually pauses.
                ((DocTabPage)_TabDocs.SelectedTab).CodeEditor.Focus();
            }
            WriteToCmdPipe("::Pause~");
        }
    }
}
