using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using SSIDE.Forms;

namespace SSIDE.Classes
{
    public class CodeEditor : NTDLS.Windows.Forms.CodeEditorControl
    {
        public DocTabPage _tabPage;
        public frmMain _OwnerForm;
        public bool AllowAutoComplete = false;

        /// <summary>
        /// Creates a new file.
        /// </summary>
        public CodeEditor(frmMain ownerForm, DocTabPage tabPage, string tabText)
        {
            InitializeComponent();

            _OwnerForm = ownerForm;
            _tabPage = tabPage;
            _tabPage.Text = tabText;

            this.Document = new NTDLS.Syntax.SyntaxDocument();
            this.Name = "CodeEditor";
            this.BracketMatching = true;
            this.Dock = DockStyle.Fill;
            this.ShowLineNumbers = true;
            this.ShowScopeIndicator = true;
            this.ShowTabGuides = true;
            this.AllowDrop = true;
            //FIXFIX: Auto indent.
            //this.Indent = NTDLS.Windows.Forms.CodeEditor.IndentStyle.Scope;
            //this.TabSize = 4;

            //SetSyntaxHighlighter("SSE");
        }

        public void SetSyntaxHighlighter(string highlighterName)
        {
            string syntaxtFileName = global.GetRegistryString("", "Path") + "\\IDE\\Highlighters\\" + highlighterName + ".syn";

            AllowAutoComplete = (highlighterName.ToLower() == "sse");

            try
            {
                this.Document.SyntaxFile = syntaxtFileName;
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
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();
            // 
            // CodeEditor
            // 
            this.RowClick += new NTDLS.Windows.Forms.CodeEditor.RowMouseHandler(this.CodeEditor_RowClick);
            this.ResumeLayout(false);
        }

        private void CodeEditor_RowClick(object sender, NTDLS.Windows.Forms.CodeEditor.RowMouseEventArgs e)
        {
            this.HighLightActiveLine = false;
        }
    }
}
