using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using SSIDE.Forms;

namespace SSIDE.Classes
{
    public class DocTabPage : TabPage
    {
        private CodeEditor _CodeEditor;
        private string _TempFileName;

        public CodeEditor CodeEditor
        {
            get { return this._CodeEditor; }
            set { this._CodeEditor = value; }
        }

        public string TempFileName
        {
            get { return this._TempFileName; }
            set { this._TempFileName = value; }
        }
    }
}
