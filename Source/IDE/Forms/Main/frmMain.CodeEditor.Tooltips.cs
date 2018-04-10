using System;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        public delegate void deToolTipSymbolInfo(String sName, String sType, String sValue);

        public void ToolTipSymbolInfo(String sName, String sType, String sValue)
        {
            _RunningApplication.TipMenuItem.DropDownItems.Clear();
            _RunningApplication.TipMenuItem.DropDownItems.Add("Name: " + sName);
            _RunningApplication.TipMenuItem.DropDownItems.Add("Type: " + sType);
            _RunningApplication.TipMenuItem.DropDownItems.Add("Value: " + sValue);
        }
    }
}
