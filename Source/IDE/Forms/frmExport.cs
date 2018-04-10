using System;
using System.Collections;
using System.IO;
using System.Windows.Forms;
using SSIDE.Classes;


namespace SSIDE.Forms
{
    public partial class frmExport : Form
    {
        DocTabPage _DocTabPage;

        public frmExport(DocTabPage docTabPage)
        {
            InitializeComponent();
            _DocTabPage = docTabPage;

            if (docTabPage.CodeEditor.FileName != null && docTabPage.CodeEditor.FileName.Trim().Length > 0)
            {
                txtExportLocation.Text =
                    Path.GetDirectoryName(docTabPage.CodeEditor.FileName)
                    + "\\" + Path.GetFileNameWithoutExtension(docTabPage.CodeEditor.FileName) + ".htm";
            }
            else
            {
                txtExportLocation.Text =
                    Environment.GetFolderPath(Environment.SpecialFolder.Desktop)
                    + "\\" + Path.GetFileNameWithoutExtension(_DocTabPage.Text) + ".htm";
            }

            txtExportLocation.Text = RemoveExcessBackslashes(txtExportLocation.Text);
            txtRelativeImageURI.Text = global.GetRegistryString("IDE", "LastExportImagesURI");

            if (txtRelativeImageURI.Text.Trim().Length == 0)
            {
                txtRelativeImageURI.Text = "/images";
            }
        }

        private void frmExport_Load(object sender, EventArgs e)
        {
            this.AcceptButton = cmdExport;
            this.CancelButton = cmdCancel;
        }

        private void cmdCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            Close();
        }

        private void cmdExport_Click(object sender, EventArgs e)
        {
            if (txtExportLocation.Text.Trim().Length == 0)
            {
                MessageBox.Show("You must specify a target file for the export.");
                return;
            }

            if (txtRelativeImageURI.Text.Trim().Length > 0)
            {
                if(txtRelativeImageURI.Text.Substring(txtRelativeImageURI.Text.Length - 1) != "/")
                {
                    txtRelativeImageURI.Text += "/";
                }
            }

            global.SetRegistryString("IDE", "LastExportImagesURI", txtRelativeImageURI.Text);

            NTDLS.Syntax.SyntaxDocumentExporters.HTMLExporter htmlExporter = new NTDLS.Syntax.SyntaxDocumentExporters.HTMLExporter();

            string htmlText = htmlExporter.Export(_DocTabPage.CodeEditor.Document, txtRelativeImageURI.Text);
            try
            {
                using (System.IO.StreamWriter outfile = new System.IO.StreamWriter(txtExportLocation.Text))
                {
                    string sourceFolder = global.GetRegistryString("", "Path") + "\\IDE\\HTML Code Images";

                    string targetFolder = RemoveExcessBackslashes(Path.GetDirectoryName(txtExportLocation.Text) + "\\" + txtRelativeImageURI.Text);

                    CopyFiles(sourceFolder, targetFolder);

                    outfile.Write(htmlText);
                }
            }
            catch(Exception ex)
            {
                MessageBox.Show("Failed to export file: " + ex.Message);
                return;
            }

            MessageBox.Show("Export successful!");

            this.DialogResult = DialogResult.OK;
            Close();
        }

        void CreateFolderStructure(string targetFolder)
        {
            for (int lastSlash = 0; (lastSlash = targetFolder.IndexOf('\\', lastSlash)) > 0; lastSlash++)
            {
                try
                {
                    System.IO.Directory.CreateDirectory(targetFolder.Substring(0, lastSlash));
                }
                catch
                {
                }
            }

            try
            {
                System.IO.Directory.CreateDirectory(targetFolder);
            }
            catch
            {
            }
        }

        void CopyFiles(string sourceFolder, string targetFolder)
        {
            ArrayList FoldersFiles = new ArrayList();
            DirectoryInfo folder = new DirectoryInfo(sourceFolder);

            CreateFolderStructure(targetFolder);

            if (folder.Exists)
            {
                string[] files = Directory.GetFiles(sourceFolder);
                foreach (string fileName in files)
                {
                    FileInfo fileInfo = new FileInfo(fileName);

                    System.IO.File.Copy(fileName, targetFolder + "\\" + fileInfo.Name, true);
                }
            }
        }

        string RemoveExcessBackslashes(string inputPath)
        {
            inputPath = inputPath.Replace("/", "\\");

            int previousLength = 0;
            do
            {
                previousLength = inputPath.Length;
                inputPath = inputPath.Replace("\\\\", "\\");
            } while (previousLength > inputPath.Length);

            return inputPath;
        }

        private void cmdSelectLocation_Click(object sender, EventArgs e)
        {
            System.Windows.Forms.SaveFileDialog ofdScript = new System.Windows.Forms.SaveFileDialog();

            ofdScript.ShowHelp = false;
            ofdScript.DefaultExt = ".htm";
            ofdScript.CheckPathExists = true;
            ofdScript.ValidateNames = true;
            ofdScript.Filter = "HTML|*html|Text Files|*.txt|All Files|*.*";
            ofdScript.FileName = txtExportLocation.Text;

            if (ofdScript.ShowDialog() == DialogResult.OK)
            {
                txtExportLocation.Text = ofdScript.FileName;
            }
        }
    }
}
