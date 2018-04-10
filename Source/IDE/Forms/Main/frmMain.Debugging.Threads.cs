using System;
using System.Diagnostics;
using System.IO.Pipes;
using System.Text;
using System.Windows.Forms;
using SSIDE.Classes;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        public delegate void deExecutionComplete();
        public delegate void deExecutionBegin();

        void ProcessMonitorThread(Object oThreadParam)
        {
            if (_RunningApplication.WasAttach)
            {
                //If we're attaching, then we obviously do not want to start a process.
                _RunningApplication.Process = Process.GetProcessById(_RunningApplication.AttachProcessId);
            }
            else
            {
                //We're starting a debug process, so goahead and launch the new process.
                ProcessStartInfo processStartInfo = new ProcessStartInfo("\"" + _RunningApplication.Engine + "\"");

                _RunningApplication.Form.Invoke(_RunningApplication.pExecutionBegin);

                processStartInfo.UseShellExecute = false;
                if (_IDEOptions.DebugShowConsole)
                {
                    processStartInfo.WindowStyle = ProcessWindowStyle.Normal;
                    processStartInfo.CreateNoWindow = false;
                    processStartInfo.Arguments = "\"" + _RunningApplication.Script + "\" /Pause /Debug:" + _RunningApplication.InstanceId;
                }
                else
                {
                    processStartInfo.WindowStyle = ProcessWindowStyle.Hidden;
                    processStartInfo.CreateNoWindow = true;
                    processStartInfo.Arguments = "\"" + _RunningApplication.Script + "\" /Debug:" + _RunningApplication.InstanceId;
                }

                _RunningApplication.Process = Process.Start(processStartInfo);
            }

            try
            {
                _RunningApplication.WriteCmdPipe = new NamedPipeClientStream(
                    ".", "SSE_RCMD_" + _RunningApplication.InstanceId, PipeDirection.Out, PipeOptions.WriteThrough);
                _RunningApplication.WriteCmdPipe.Connect(5000);

                _RunningApplication.ReadCmdPipe = new NamedPipeClientStream(
                    ".", "SSE_WCMD_" + _RunningApplication.InstanceId, PipeDirection.In, PipeOptions.WriteThrough);
                _RunningApplication.ReadCmdPipe.Connect(5000);

                _RunningApplication.ReadTxtPipe = new NamedPipeClientStream(
                    ".", "SSE_WTXT_" + _RunningApplication.InstanceId, PipeDirection.In, PipeOptions.WriteThrough);
                _RunningApplication.ReadTxtPipe.Connect(5000);

                _RunningApplication.ReadErrPipe = new NamedPipeClientStream(
                    ".", "SSE_WERR_" + _RunningApplication.InstanceId, PipeDirection.In, PipeOptions.WriteThrough);
                _RunningApplication.ReadErrPipe.Connect(5000);

                _RunningApplication.NamedPipeThreadCmd = new System.Threading.Thread(CmdNamedPipe_Thread);
                _RunningApplication.NamedPipeThreadCmd.Start();

                _RunningApplication.NamedPipeThreadTxt = new System.Threading.Thread(TxtNamedPipe_Thread);
                _RunningApplication.NamedPipeThreadTxt.Start();

                _RunningApplication.NamedPipeThreadErr = new System.Threading.Thread(ErrNamedPipe_Thread);
                _RunningApplication.NamedPipeThreadErr.Start();

                DumpInitialBreakpoints();

                //Wait for exit, but also wait on [IsRunning] because we may just be atached
                //  in which case {IsRunning == false} means that the user requested a detach.
                while (_RunningApplication.IsRunning && _RunningApplication.Process.HasExited == false)
                {
                    System.Threading.Thread.Sleep(100);
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }

            _RunningApplication.IsRunning = false;
            _RunningApplication.Form.Invoke(_RunningApplication.pExecutionComplete);
        }

        void Process_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            if (e != null && e.Data != null)
            {
                _RunningApplication.Form.Invoke(_RunningApplication.pAddOutputToList, e.Data + "\r\n");
            }
        }

        void ErrNamedPipe_Thread(Object oThreadParam)
        {
            try
            {
                byte[] Bytes = new byte[1024];
                int iBytes = 0;

                while (_RunningApplication.IsRunning)
                {
                    iBytes = _RunningApplication.ReadErrPipe.Read(Bytes, 0, 1024);
                    if (iBytes > 0)
                    {
                        System.Text.ASCIIEncoding Decoder = new System.Text.ASCIIEncoding();

                        string sBuffer = Decoder.GetString(Bytes, 0, iBytes);

                        foreach (string Line in sBuffer.Split('\r'))
                        {
                            _RunningApplication.Form.Invoke(_RunningApplication.pAddErrorToList, Line);
                        }
                    }
                }
            }
            catch
            {
            }
        }

        void TxtNamedPipe_Thread(Object oThreadParam)
        {
            try
            {
                byte[] Bytes = new byte[1024];
                int iBytes = 0;

                while (_RunningApplication.IsRunning)
                {
                    iBytes = _RunningApplication.ReadTxtPipe.Read(Bytes, 0, 1024);
                    if (iBytes > 0)
                    {
                        System.Text.ASCIIEncoding Decoder = new System.Text.ASCIIEncoding();

                        string sBuffer = Decoder.GetString(Bytes, 0, iBytes);

                        _RunningApplication.Form.Invoke(_RunningApplication.pAddOutputToList, sBuffer);
                    }
                }
            }
            catch
            {
            }
        }

        void CmdNamedPipe_Thread(Object oThreadParam)
        {
            try
            {
                byte[] Bytes = new byte[1024];
                int iBytes = 0;

                while (_RunningApplication.IsRunning)
                {
                    iBytes = _RunningApplication.ReadCmdPipe.Read(Bytes, 0, 1024);
                    if (iBytes > 0)
                    {
                        System.Text.ASCIIEncoding Decoder = new System.Text.ASCIIEncoding();

                        string sBuffer = Decoder.GetString(Bytes, 0, iBytes);

                        string[] sArray = sBuffer.Split('|');

                        //-------------------------------------------------------------------------------------------------------
                        if (sArray[0].ToLower() == "::EndOfLocalVariable~".ToLower())
                        {
                            _RunningApplication.Form.Invoke(_RunningApplication.pRemoveNonUpdatedLocals);
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::UpdateLocalVariable~".ToLower())
                        {
                            System.Text.StringBuilder sValue = new System.Text.StringBuilder();

                            //The remaining are symbol item text.
                            for (int iIndex = 4; iIndex < sArray.Length; iIndex++)
                            {
                                sValue.Append(sArray[iIndex]);
                            }

                            _RunningApplication.Form.Invoke(_RunningApplication.pUpdateLocalsValue, sArray[1], int.Parse(sArray[2]), sArray[3], sValue.ToString());
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::BeginListVariables~".ToLower())
                        {
                            _WatchForm.cboExpression.Items.Clear();
                            _WatchForm.Enabled = false;
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::ListVariables~".ToLower())
                        {
                            for (int iIndex = 1; iIndex < sArray.Length; iIndex++)
                            {
                                if (sArray[iIndex] != "")
                                {
                                    _WatchForm.cboExpression.Items.Add(sArray[iIndex]);
                                }
                            }
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::EndListVariables~".ToLower())
                        {
                            _WatchForm.Enabled = true;
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::BeginFileList~".ToLower())
                        {
                            _RunningApplication.Form.Invoke(_RunningApplication.pClearFilesGrid);
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::FileList~".ToLower())
                        {
                            _RunningApplication.Form.Invoke(_RunningApplication.pAddFileToGrid, sArray[1]);
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::EndFileList~".ToLower())
                        {
                            _RunningApplication.Form.Invoke(_RunningApplication.pAutosizeFileGrid);
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::BreakPointHit~".ToLower())
                        {
                            System.Text.StringBuilder sValue = new System.Text.StringBuilder();

                            //The remaining items are line item.
                            for (int iIndex = 3; iIndex < sArray.Length; iIndex++)
                            {
                                sValue.Append(sArray[iIndex]);
                            }

                            _RunningApplication.Form.Invoke(_RunningApplication.pBreakPointHit, sArray[1], int.Parse(sArray[2]) - 1, sValue.ToString());
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::ToolTipSymbolInfo~".ToLower())
                        {
                            System.Text.StringBuilder sValue = new System.Text.StringBuilder();

                            //The remaining are symbol item text.
                            for (int iIndex = 3; iIndex < sArray.Length; iIndex++)
                            {
                                sValue.Append(sArray[iIndex]);
                            }

                            _RunningApplication.Form.Invoke(_RunningApplication.pToolTipSymbolInfo, sArray[1], sArray[2], sValue.ToString());
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::QuickWatch~".ToLower())
                        {
                            System.Text.StringBuilder sValue = new System.Text.StringBuilder();

                            //The remaining are symbol item text.
                            for (int iIndex = 3; iIndex < sArray.Length; iIndex++)
                            {
                                sValue.Append(sArray[iIndex]);
                            }

                            _RunningApplication.Form.Invoke(_RunningApplication.pQuickWatchInfo, sArray[1], sArray[2], sValue.ToString());
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::ImmediateAutoListBegin~".ToLower())
                        {
                            _RunningApplication.Form.Invoke(_RunningApplication.pImmediateAutoListBegin);
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::ImmediateAutoListEnd~".ToLower())
                        {
                            _RunningApplication.Form.Invoke(_RunningApplication.pImmediateAutoListEnd);
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::ImmediateWord~".ToLower())
                        {
                            _RunningApplication.Form.Invoke(_RunningApplication.pImmediateAutoListAddWord, sArray[1], sArray[2]);
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::ImmediateInfo~".ToLower())
                        {
                            System.Text.StringBuilder sValue = new System.Text.StringBuilder();

                            //The remaining are symbol item text.
                            for (int iIndex = 1; iIndex < sArray.Length; iIndex++)
                            {
                                sValue.Append(sArray[iIndex]);
                            }

                            _RunningApplication.Form.Invoke(_RunningApplication.pAddImmediateInfo, sValue.ToString());
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else if (sArray[0].ToLower() == "::SymbolInfo~".ToLower())
                        {
                            System.Text.StringBuilder sValue = new System.Text.StringBuilder();

                            //The remaining are symbol item text.
                            for (int iIndex = 3; iIndex < sArray.Length; iIndex++)
                            {
                                sValue.Append(sArray[iIndex]);
                            }

                            _RunningApplication.Form.Invoke(_RunningApplication.pUpdateWatchValue, sArray[1], sArray[2], sValue.ToString());
                        }
                        //-------------------------------------------------------------------------------------------------------
                        else
                        {
                            MessageBox.Show("Unknown debugger command: " + sArray[0]);
                        }

                        System.Threading.Thread.Sleep(1);
                    }
                }
            }
            catch//(Exception ex)
            {
                //MessageBox.Show(ex.Message);
            }
        }

        public void ExecutionComplete()
        {
            SetIconsStopped();

            _ErrorGrid.AutoResizeColumns(DataGridViewAutoSizeColumnsMode.AllCells);

            if (_ErrorGrid.Rows.Count > 0)
            {
                _TabTools.SelectedTab = tabErrors;
            }
            else if (_OutputBox.Text.Length > 0)
            {
                _TabTools.SelectedTab = tabOutput;
            }
            else if (_WatchGrid.Rows.Count > 0)
            {
                _TabTools.SelectedTab = tabWatch;
            }

            if (_RunningApplication.OriginalTab != null && _RunningApplication.OriginalTab.TempFileName != null)
            {
                System.IO.File.Delete(_RunningApplication.OriginalTab.TempFileName);
                _RunningApplication.OriginalTab.CodeEditor.FileName = null;
                _RunningApplication.OriginalTab.CodeEditor.Saved = false;
            }

            if (CurrentTab != null)
            {
                CurrentTab.Focus();
                CurrentTab.CodeEditor.Focus();
            }

            if (_RunningApplication.OriginalTab != null)
            {
                _TabDocs.SelectedTab = _RunningApplication.OriginalTab;
            }
        }

        public void ExecutionBegin()
        {
            foreach (DocTabPage tab in _TabDocs.TabPages)
            {
                tab.CodeEditor.HighLightActiveLine = false;

                if (!tab.CodeEditor.Saved && tab.CodeEditor.FileName != null)
                {
                    tab.CodeEditor.Save();
                }
            }

            _TabTools.SelectedTab = tabOutput;
            _LocalsGrid.Rows.Clear();
        }

        void WriteToCmdPipe(string data)
        {
            try
            {
                System.Text.ASCIIEncoding Encoder = new System.Text.ASCIIEncoding();
                byte[] Bytes = Encoder.GetBytes(data);

                _RunningApplication.WriteCmdPipe.Write(Bytes, 0, Bytes.Length);
                _RunningApplication.WriteCmdPipe.Flush();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                Debug_Stop();
            }
        }
    }
}
