using System;
using System.Drawing;
using SSIDE.Classes;
using System.IO;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        public delegate void deBreakPointHit(String sFileName, int iRowIndex, string lineText);

        void DumpInitialBreakpoints()
        {
            //Dump in the break points.
            WriteToCmdPipe("::BreakPoints:Begin~");

            foreach (DocTabPage tab in _TabDocs.TabPages)
            {
                for (int iIndex = 0; iIndex < tab.CodeEditor.Document.Count; iIndex++)
                {
                    if (tab.CodeEditor.Document[iIndex].Breakpoint)
                    {
                        WriteToCmdPipe("::BreakPoint~|" + tab.CodeEditor.FileName + "|" + (iIndex + 1));
                    }
                }
            }

            WriteToCmdPipe("::BreakPoints:End~");
        }

        public void BreakPointHit(String sFileName, int iRowIndex, string lineText)
        {
            FileInfo fileInfo = new FileInfo(sFileName);
            sFileName = fileInfo.FullName; //Removes ".." and "." also known as PathCanonicalize().

            do
            {
                foreach (DocTabPage tab in _TabDocs.TabPages)
                {
                    if (tab.CodeEditor.FileName != null && tab.CodeEditor.FileName.ToLower() == sFileName.ToLower())
                    {
                        _TabDocs.SelectedTab = tab;

                        SetIconsPaused();

                        _RunningApplication.ExecuteLine = tab.CodeEditor.Document[iRowIndex];
                        tab.CodeEditor.GotoLine(iRowIndex);
                        _RunningApplication.ExecuteLine.BackColor = Color.Pink;

                        _DebugLineText.Text = lineText.Replace("&", "&&");

                        UpdateAllWatchValues();
                        UpdateAllLocalVariables();
                        UpdateAllLoadedFiles();
                        return;
                    }
                }
            }
            while (AddNewTab(sFileName) != null);
        }
    }
}
