using System;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;
using SSIDE.Classes;

namespace SSIDE.Forms
{
    public partial class frmAbout : Form
    {
        public frmAbout()
        {
            InitializeComponent();
        }

        private void frmAbout_Load(object sender, EventArgs e)
        {
            String sEngine = global.GetRegistryString("", "Path") + "\\SSE.exe";
            string sIDE = Application.ExecutablePath;

            FileVersionInfo VerIDE = FileVersionInfo.GetVersionInfo(sIDE);

            pbIcon.Image = Icon.ExtractAssociatedIcon(Application.ExecutablePath).ToBitmap();
            lblIDEApp.Text = VerIDE.ProductName;
            lblIDEVer.Text = VerIDE.FileVersion;
            lblIDECopyRight.Text = VerIDE.LegalCopyright;

            try {
                FileVersionInfo VerEngine = FileVersionInfo.GetVersionInfo(sEngine);

                lblEngineApp.Text = VerEngine.ProductName;
                lblEngineVer.Text = VerEngine.FileVersion;
                lblEngineCopyRight.Text = VerEngine.LegalCopyright;
            }
            catch {
                lblEngineApp.Text = "<not installed>";
                lblEngineVer.Text = "<not installed>";
                lblEngineCopyRight.Text = "<not installed>";
            }

            this.AcceptButton = cmdOk;
            this.CancelButton = cmdOk;
        }
    }
}
