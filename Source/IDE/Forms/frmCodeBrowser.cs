using System;
using System.IO;
using System.Windows.Forms;
using SSIDE.Classes;

namespace SSIDE.Forms
{
    public partial class frmCodeBrowser : Form
    {
        public string Filename { get; set; }
        private bool _SnippetsMode = false;
        Button _CloseButton = new Button();

        public frmCodeBrowser()
        {
            InitializeComponent();

            _CloseButton.Click += new EventHandler(closeButton_Click);
            this.CancelButton = _CloseButton;
        }

        public frmCodeBrowser(string path, bool snippetsMode, string title, string description)
        {
            InitializeComponent();

            _CloseButton.Click += new EventHandler(closeButton_Click);
            this.CancelButton = _CloseButton;

            _LibraryLocation.Text = path;
            _SnippetsMode = snippetsMode;
            lblDescriptionValue.Text = description;
            this.Text = title;
        }

        void closeButton_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }

        private void frmCodeBrowser_Load(object sender, EventArgs e)
        {
            string syntaxtFileName = global.GetRegistryString("", "Path") + "\\IDE\\Highlighters\\SSE.syn";

            _CodeEditor.ReadOnly = true;
            _CodeEditor.ShowTabGuides = false;
            _CodeEditor.AllowBreakPoints = false;
            _CodeEditor.ShowLineNumbers = true;

            try
            {
                _CodeEditor.Document.SyntaxFile = syntaxtFileName;
            }
            catch (Exception ex)
            {
                if (ex.GetType() == typeof(System.IO.DirectoryNotFoundException))
                {
                    MessageBox.Show("The syntax file path could not be found. Highlighting will not be available.\r\n\"" + syntaxtFileName + "\"");
                }
                else
                {
                    throw ex;
                }
            }

            PopList();
        }

        public string CodeText
        {
            get
            {
                StreamReader textReader = new StreamReader(this.Filename);
                string value = textReader.ReadToEnd();
                textReader.Close();
                DialogResult = DialogResult.OK;
                Close();
                return value;
            }
        }

        private void PopList()
        {
            string[] fileNames = Directory.GetFiles(_LibraryLocation.Text, "*.ss");

           foreach (string fileName in fileNames)
           {
               _FileList.Items.Add(System.IO.Path.GetFileNameWithoutExtension(fileName));
           }

           if (_FileList.Items.Count > 0)
           {
               _FileList.SelectedIndex = 0;
           }
        }

        private void _FileList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (_FileList.SelectedItem != null)
            {
                string fileName = _LibraryLocation.Text + "\\" + _FileList.SelectedItem.ToString() + ".ss";

                StreamReader textReader = new StreamReader(fileName);
                string codeText = textReader.ReadToEnd();
                textReader.Close();

                if (_SnippetsMode)
                {
                    _CodeEditor.Document.Text = "<%\r\n\r\n" + codeText + "\r\n\r\n%>";
                }
                else
                {
                    _CodeEditor.Document.Text = codeText;
                }
            }
        }

        private void _FileList_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (_FileList.SelectedItem != null)
            {
                this.Filename = _LibraryLocation.Text + "\\" + _FileList.SelectedItem.ToString() + ".ss";
                DialogResult = DialogResult.OK;
                Close();
            }
        }

        private void cmdBrowse_Click(object sender, EventArgs e)
        {
            System.Diagnostics.Process process = new System.Diagnostics.Process();
            process.StartInfo.UseShellExecute = true;
            process.StartInfo.FileName = "explorer";
            process.StartInfo.Arguments = _LibraryLocation.Text;
            process.Start();
        }
    }
}
