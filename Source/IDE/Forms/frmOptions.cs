using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using SSIDE.Classes;

namespace SSIDE.Forms
{
    public partial class frmOptions : Form
    {
        public IDEOptions _IDEOptions;

        public frmOptions(IDEOptions ideOptions)
        {
            InitializeComponent();
            _IDEOptions = ideOptions;
        }

        private void frmOptions_Load(object sender, EventArgs e)
        {
            string filePath = global.GetRegistryString("", "Path");
            string syntaxtFileName = filePath + "\\IDE\\Highlighters\\SSE.syn";

            _CodeEditor.ReadOnly = false;
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

            cbBracketMatching.Checked = _IDEOptions.BracketMatching;
            cbShowScopeIndicator.Checked = _IDEOptions.ShowScopeIndicator;
            cbShowWhitespace.Checked = _IDEOptions.ShowWhitespace;
            cbShowEOLMarker.Checked = _IDEOptions.ShowEOLMarker;
            cbShowGutterMargin.Checked = _IDEOptions.ShowGutterMargin;
            cbShowLineNumbers.Checked = _IDEOptions.ShowLineNumbers;
            cbEnableAutoComplete.Checked = _IDEOptions.EnableAutoComplete;
            cbAutoCompleteSimpleExpressions.Checked = _IDEOptions.AutoCompleteSimpleExpressions;
            cbAutoCompleteComplexExpressions.Checked = _IDEOptions.AutoCompleteComplexExpressions;
            cbAutoCompleteMatchOnBeginningOnly.Checked = _IDEOptions.AutoCompleteMatchOnBeginningOnly;
            cbDebugShowConsole.Checked = _IDEOptions.DebugShowConsole;
            cbEnableCodeFolding.Checked = _IDEOptions.EnableCodeFolding;
            _CodeEditor.Document.Text = _IDEOptions.DefaultText;
            _CodeEditor.ScrollIntoView(0);
            _CodeEditor.Caret.Position.X = _IDEOptions.DefaultCaretX;
            _CodeEditor.Caret.Position.Y = _IDEOptions.DefaultCaretY;
        }

        private void cmdOk_Click(object sender, EventArgs e)
        {
            _IDEOptions.BracketMatching = cbBracketMatching.Checked;
            _IDEOptions.ShowScopeIndicator = cbShowScopeIndicator.Checked;
            _IDEOptions.ShowWhitespace = cbShowWhitespace.Checked;
            _IDEOptions.ShowEOLMarker = cbShowEOLMarker.Checked;
            _IDEOptions.ShowGutterMargin = cbShowGutterMargin.Checked;
            _IDEOptions.ShowLineNumbers = cbShowLineNumbers.Checked;
            _IDEOptions.EnableAutoComplete = cbEnableAutoComplete.Checked;
            _IDEOptions.AutoCompleteSimpleExpressions = cbAutoCompleteSimpleExpressions.Checked;
            _IDEOptions.AutoCompleteComplexExpressions = cbAutoCompleteComplexExpressions.Checked;
            _IDEOptions.AutoCompleteMatchOnBeginningOnly = cbAutoCompleteMatchOnBeginningOnly.Checked;
            _IDEOptions.DebugShowConsole = cbDebugShowConsole.Checked;
            _IDEOptions.EnableCodeFolding = cbEnableCodeFolding.Checked;
            _IDEOptions.DefaultText = _CodeEditor.Document.Text;
            _IDEOptions.DefaultCaretX = _CodeEditor.Caret.Position.X;
            _IDEOptions.DefaultCaretY = _CodeEditor.Caret.Position.Y;

            _IDEOptions.SaveToRegistry();
            this.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.Close();
        }

        private void cmdCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Close();
        }
    }
}
