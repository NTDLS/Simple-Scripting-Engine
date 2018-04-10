using System;
using System.Windows.Forms;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        public delegate void deAddOutputToList(String sText);

        public void AddOutputToList(String sText)
        {
            _OutputBox.AppendText(sText);
        }

        private void _OutputBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Control == true && e.KeyCode == Keys.A)
            {
                _OutputBox.SelectAll();
                e.SuppressKeyPress = true;
            }
            else if (e.KeyCode == Keys.Enter)
            {
                int iSelStart = _OutputBox.SelectionStart;

                int iCodeLine = _OutputBox.GetLineFromCharIndex(iSelStart);

                if (_OutputBox.Lines[iCodeLine].Length > 0)
                {
                    if (_RunningApplication.IsRunning)
                    {
                        WriteToCmdPipe("::Immediate~|" + _OutputBox.Lines[iCodeLine]);
                    }
                    else
                    {
                        AddImmediateInfo("<Not Running>");
                    }
                }

                e.SuppressKeyPress = true;
            }
        }
    }
}
