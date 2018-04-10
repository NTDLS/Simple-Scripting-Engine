using System;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        private void _TabTools_Click(object sender, EventArgs e)
        {
            if (_TabTools.SelectedTab == tabImmediate)
            {
                _ImmediateText.Focus();
            }
        }
    }
}
