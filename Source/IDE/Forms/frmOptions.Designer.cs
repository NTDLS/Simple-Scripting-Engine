namespace SSIDE.Forms
{
    partial class frmOptions
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            NTDLS.Windows.Forms.LineMarginRender lineMarginRender1 = new NTDLS.Windows.Forms.LineMarginRender();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frmOptions));
            this.cmdOk = new System.Windows.Forms.Button();
            this.cmdCancel = new System.Windows.Forms.Button();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabEditing = new System.Windows.Forms.TabPage();
            this.cbEnableCodeFolding = new System.Windows.Forms.CheckBox();
            this.cbShowLineNumbers = new System.Windows.Forms.CheckBox();
            this.cbShowGutterMargin = new System.Windows.Forms.CheckBox();
            this.cbShowEOLMarker = new System.Windows.Forms.CheckBox();
            this.cbShowScopeIndicator = new System.Windows.Forms.CheckBox();
            this.cbShowWhitespace = new System.Windows.Forms.CheckBox();
            this.cbBracketMatching = new System.Windows.Forms.CheckBox();
            this.tabAutoComplete = new System.Windows.Forms.TabPage();
            this.cbAutoCompleteMatchOnBeginningOnly = new System.Windows.Forms.CheckBox();
            this.cbAutoCompleteSimpleExpressions = new System.Windows.Forms.CheckBox();
            this.cbAutoCompleteComplexExpressions = new System.Windows.Forms.CheckBox();
            this.cbEnableAutoComplete = new System.Windows.Forms.CheckBox();
            this.tabDebug = new System.Windows.Forms.TabPage();
            this.cbDebugShowConsole = new System.Windows.Forms.CheckBox();
            this.tabDefaultCode = new System.Windows.Forms.TabPage();
            this._CodeEditor = new NTDLS.Windows.Forms.CodeEditorControl();
            this.syntaxDocument1 = new NTDLS.Syntax.SyntaxDocument(this.components);
            this.tabControl1.SuspendLayout();
            this.tabEditing.SuspendLayout();
            this.tabAutoComplete.SuspendLayout();
            this.tabDebug.SuspendLayout();
            this.tabDefaultCode.SuspendLayout();
            this.SuspendLayout();
            // 
            // cmdOk
            // 
            this.cmdOk.Location = new System.Drawing.Point(316, 238);
            this.cmdOk.Name = "cmdOk";
            this.cmdOk.Size = new System.Drawing.Size(75, 23);
            this.cmdOk.TabIndex = 0;
            this.cmdOk.Text = "Ok";
            this.cmdOk.UseVisualStyleBackColor = true;
            this.cmdOk.Click += new System.EventHandler(this.cmdOk_Click);
            // 
            // cmdCancel
            // 
            this.cmdCancel.Location = new System.Drawing.Point(397, 238);
            this.cmdCancel.Name = "cmdCancel";
            this.cmdCancel.Size = new System.Drawing.Size(75, 23);
            this.cmdCancel.TabIndex = 1;
            this.cmdCancel.Text = "Cancel";
            this.cmdCancel.UseVisualStyleBackColor = true;
            this.cmdCancel.Click += new System.EventHandler(this.cmdCancel_Click);
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabEditing);
            this.tabControl1.Controls.Add(this.tabAutoComplete);
            this.tabControl1.Controls.Add(this.tabDebug);
            this.tabControl1.Controls.Add(this.tabDefaultCode);
            this.tabControl1.Location = new System.Drawing.Point(5, 5);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(467, 227);
            this.tabControl1.TabIndex = 0;
            // 
            // tabEditing
            // 
            this.tabEditing.Controls.Add(this.cbEnableCodeFolding);
            this.tabEditing.Controls.Add(this.cbShowLineNumbers);
            this.tabEditing.Controls.Add(this.cbShowGutterMargin);
            this.tabEditing.Controls.Add(this.cbShowEOLMarker);
            this.tabEditing.Controls.Add(this.cbShowScopeIndicator);
            this.tabEditing.Controls.Add(this.cbShowWhitespace);
            this.tabEditing.Controls.Add(this.cbBracketMatching);
            this.tabEditing.Location = new System.Drawing.Point(4, 22);
            this.tabEditing.Name = "tabEditing";
            this.tabEditing.Padding = new System.Windows.Forms.Padding(3);
            this.tabEditing.Size = new System.Drawing.Size(459, 201);
            this.tabEditing.TabIndex = 0;
            this.tabEditing.Text = "Editing";
            this.tabEditing.UseVisualStyleBackColor = true;
            // 
            // cbEnableCodeFolding
            // 
            this.cbEnableCodeFolding.AutoSize = true;
            this.cbEnableCodeFolding.Location = new System.Drawing.Point(6, 145);
            this.cbEnableCodeFolding.Name = "cbEnableCodeFolding";
            this.cbEnableCodeFolding.Size = new System.Drawing.Size(124, 17);
            this.cbEnableCodeFolding.TabIndex = 6;
            this.cbEnableCodeFolding.Text = "Enable Code Folding";
            this.cbEnableCodeFolding.UseVisualStyleBackColor = true;
            // 
            // cbShowLineNumbers
            // 
            this.cbShowLineNumbers.AutoSize = true;
            this.cbShowLineNumbers.Location = new System.Drawing.Point(6, 122);
            this.cbShowLineNumbers.Name = "cbShowLineNumbers";
            this.cbShowLineNumbers.Size = new System.Drawing.Size(121, 17);
            this.cbShowLineNumbers.TabIndex = 5;
            this.cbShowLineNumbers.Text = "Show Line Numbers";
            this.cbShowLineNumbers.UseVisualStyleBackColor = true;
            // 
            // cbShowGutterMargin
            // 
            this.cbShowGutterMargin.AutoSize = true;
            this.cbShowGutterMargin.Location = new System.Drawing.Point(6, 75);
            this.cbShowGutterMargin.Name = "cbShowGutterMargin";
            this.cbShowGutterMargin.Size = new System.Drawing.Size(120, 17);
            this.cbShowGutterMargin.TabIndex = 3;
            this.cbShowGutterMargin.Text = "Show Gutter Margin";
            this.cbShowGutterMargin.UseVisualStyleBackColor = true;
            // 
            // cbShowEOLMarker
            // 
            this.cbShowEOLMarker.AutoSize = true;
            this.cbShowEOLMarker.Location = new System.Drawing.Point(6, 98);
            this.cbShowEOLMarker.Name = "cbShowEOLMarker";
            this.cbShowEOLMarker.Size = new System.Drawing.Size(113, 17);
            this.cbShowEOLMarker.TabIndex = 4;
            this.cbShowEOLMarker.Text = "Show EOL Marker";
            this.cbShowEOLMarker.UseVisualStyleBackColor = true;
            // 
            // cbShowScopeIndicator
            // 
            this.cbShowScopeIndicator.AutoSize = true;
            this.cbShowScopeIndicator.Location = new System.Drawing.Point(6, 52);
            this.cbShowScopeIndicator.Name = "cbShowScopeIndicator";
            this.cbShowScopeIndicator.Size = new System.Drawing.Size(131, 17);
            this.cbShowScopeIndicator.TabIndex = 2;
            this.cbShowScopeIndicator.Text = "Show Scope Indicator";
            this.cbShowScopeIndicator.UseVisualStyleBackColor = true;
            // 
            // cbShowWhitespace
            // 
            this.cbShowWhitespace.AutoSize = true;
            this.cbShowWhitespace.Location = new System.Drawing.Point(6, 29);
            this.cbShowWhitespace.Name = "cbShowWhitespace";
            this.cbShowWhitespace.Size = new System.Drawing.Size(113, 17);
            this.cbShowWhitespace.TabIndex = 1;
            this.cbShowWhitespace.Text = "Show Whitespace";
            this.cbShowWhitespace.UseVisualStyleBackColor = true;
            // 
            // cbBracketMatching
            // 
            this.cbBracketMatching.AutoSize = true;
            this.cbBracketMatching.Location = new System.Drawing.Point(6, 6);
            this.cbBracketMatching.Name = "cbBracketMatching";
            this.cbBracketMatching.Size = new System.Drawing.Size(110, 17);
            this.cbBracketMatching.TabIndex = 0;
            this.cbBracketMatching.Text = "Bracket Matching";
            this.cbBracketMatching.UseVisualStyleBackColor = true;
            // 
            // tabAutoComplete
            // 
            this.tabAutoComplete.Controls.Add(this.cbAutoCompleteMatchOnBeginningOnly);
            this.tabAutoComplete.Controls.Add(this.cbAutoCompleteSimpleExpressions);
            this.tabAutoComplete.Controls.Add(this.cbAutoCompleteComplexExpressions);
            this.tabAutoComplete.Controls.Add(this.cbEnableAutoComplete);
            this.tabAutoComplete.Location = new System.Drawing.Point(4, 22);
            this.tabAutoComplete.Name = "tabAutoComplete";
            this.tabAutoComplete.Padding = new System.Windows.Forms.Padding(3);
            this.tabAutoComplete.Size = new System.Drawing.Size(459, 201);
            this.tabAutoComplete.TabIndex = 1;
            this.tabAutoComplete.Text = "Auto-Complete";
            this.tabAutoComplete.UseVisualStyleBackColor = true;
            // 
            // cbAutoCompleteMatchOnBeginningOnly
            // 
            this.cbAutoCompleteMatchOnBeginningOnly.AutoSize = true;
            this.cbAutoCompleteMatchOnBeginningOnly.Location = new System.Drawing.Point(6, 75);
            this.cbAutoCompleteMatchOnBeginningOnly.Name = "cbAutoCompleteMatchOnBeginningOnly";
            this.cbAutoCompleteMatchOnBeginningOnly.Size = new System.Drawing.Size(145, 17);
            this.cbAutoCompleteMatchOnBeginningOnly.TabIndex = 3;
            this.cbAutoCompleteMatchOnBeginningOnly.Text = "Match on Beginning Only";
            this.cbAutoCompleteMatchOnBeginningOnly.UseVisualStyleBackColor = true;
            // 
            // cbAutoCompleteSimpleExpressions
            // 
            this.cbAutoCompleteSimpleExpressions.AutoSize = true;
            this.cbAutoCompleteSimpleExpressions.Location = new System.Drawing.Point(6, 29);
            this.cbAutoCompleteSimpleExpressions.Name = "cbAutoCompleteSimpleExpressions";
            this.cbAutoCompleteSimpleExpressions.Size = new System.Drawing.Size(146, 17);
            this.cbAutoCompleteSimpleExpressions.TabIndex = 1;
            this.cbAutoCompleteSimpleExpressions.Text = "Parse Simple Expressions";
            this.cbAutoCompleteSimpleExpressions.UseVisualStyleBackColor = true;
            // 
            // cbAutoCompleteComplexExpressions
            // 
            this.cbAutoCompleteComplexExpressions.AutoSize = true;
            this.cbAutoCompleteComplexExpressions.Location = new System.Drawing.Point(6, 52);
            this.cbAutoCompleteComplexExpressions.Name = "cbAutoCompleteComplexExpressions";
            this.cbAutoCompleteComplexExpressions.Size = new System.Drawing.Size(155, 17);
            this.cbAutoCompleteComplexExpressions.TabIndex = 2;
            this.cbAutoCompleteComplexExpressions.Text = "Parse Complex Expressions";
            this.cbAutoCompleteComplexExpressions.UseVisualStyleBackColor = true;
            // 
            // cbEnableAutoComplete
            // 
            this.cbEnableAutoComplete.AutoSize = true;
            this.cbEnableAutoComplete.Location = new System.Drawing.Point(6, 6);
            this.cbEnableAutoComplete.Name = "cbEnableAutoComplete";
            this.cbEnableAutoComplete.Size = new System.Drawing.Size(131, 17);
            this.cbEnableAutoComplete.TabIndex = 0;
            this.cbEnableAutoComplete.Text = "Enable Auto-Complete";
            this.cbEnableAutoComplete.UseVisualStyleBackColor = true;
            // 
            // tabDebug
            // 
            this.tabDebug.Controls.Add(this.cbDebugShowConsole);
            this.tabDebug.Location = new System.Drawing.Point(4, 22);
            this.tabDebug.Name = "tabDebug";
            this.tabDebug.Padding = new System.Windows.Forms.Padding(3);
            this.tabDebug.Size = new System.Drawing.Size(459, 201);
            this.tabDebug.TabIndex = 2;
            this.tabDebug.Text = "Debug";
            this.tabDebug.UseVisualStyleBackColor = true;
            // 
            // cbDebugShowConsole
            // 
            this.cbDebugShowConsole.AutoSize = true;
            this.cbDebugShowConsole.Location = new System.Drawing.Point(6, 6);
            this.cbDebugShowConsole.Name = "cbDebugShowConsole";
            this.cbDebugShowConsole.Size = new System.Drawing.Size(94, 17);
            this.cbDebugShowConsole.TabIndex = 0;
            this.cbDebugShowConsole.Text = "Show Console";
            this.cbDebugShowConsole.UseVisualStyleBackColor = true;
            // 
            // tabDefaultCode
            // 
            this.tabDefaultCode.Controls.Add(this._CodeEditor);
            this.tabDefaultCode.Location = new System.Drawing.Point(4, 22);
            this.tabDefaultCode.Name = "tabDefaultCode";
            this.tabDefaultCode.Padding = new System.Windows.Forms.Padding(3);
            this.tabDefaultCode.Size = new System.Drawing.Size(459, 201);
            this.tabDefaultCode.TabIndex = 3;
            this.tabDefaultCode.Text = "Default Code";
            this.tabDefaultCode.UseVisualStyleBackColor = true;
            // 
            // _CodeEditor
            // 
            this._CodeEditor.ActiveView = NTDLS.Windows.Forms.CodeEditor.ActiveView.BottomRight;
            this._CodeEditor.AllowBreakPoints = false;
            this._CodeEditor.AutoListPosition = null;
            this._CodeEditor.AutoListSelectedText = "";
            this._CodeEditor.AutoListVisible = false;
            this._CodeEditor.CopyAsRTF = false;
            this._CodeEditor.Dock = System.Windows.Forms.DockStyle.Fill;
            this._CodeEditor.FileName = null;
            this._CodeEditor.InfoTipCount = 1;
            this._CodeEditor.InfoTipPosition = null;
            this._CodeEditor.InfoTipSelectedIndex = 1;
            this._CodeEditor.InfoTipVisible = false;
            lineMarginRender1.Bounds = new System.Drawing.Rectangle(19, 0, 19, 16);
            this._CodeEditor.LineMarginRender = lineMarginRender1;
            this._CodeEditor.Location = new System.Drawing.Point(3, 3);
            this._CodeEditor.LockCursorUpdate = false;
            this._CodeEditor.Name = "_CodeEditor";
            this._CodeEditor.Saved = false;
            this._CodeEditor.ShowScopeIndicator = false;
            this._CodeEditor.Size = new System.Drawing.Size(453, 195);
            this._CodeEditor.SmoothScroll = false;
            this._CodeEditor.SplitviewH = -4;
            this._CodeEditor.SplitviewV = -4;
            this._CodeEditor.TabGuideColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(219)))), ((int)(((byte)(214)))));
            this._CodeEditor.TabIndex = 2;
            this._CodeEditor.WhitespaceColor = System.Drawing.SystemColors.ControlDark;
            // 
            // syntaxDocument1
            // 
            this.syntaxDocument1.Lines = new string[] {
        ""};
            this.syntaxDocument1.MaxUndoBufferSize = 1000;
            this.syntaxDocument1.Modified = false;
            this.syntaxDocument1.UndoStep = 0;
            // 
            // frmOptions
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(477, 269);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.cmdCancel);
            this.Controls.Add(this.cmdOk);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "frmOptions";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Options";
            this.Load += new System.EventHandler(this.frmOptions_Load);
            this.tabControl1.ResumeLayout(false);
            this.tabEditing.ResumeLayout(false);
            this.tabEditing.PerformLayout();
            this.tabAutoComplete.ResumeLayout(false);
            this.tabAutoComplete.PerformLayout();
            this.tabDebug.ResumeLayout(false);
            this.tabDebug.PerformLayout();
            this.tabDefaultCode.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button cmdOk;
        private System.Windows.Forms.Button cmdCancel;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabEditing;
        private System.Windows.Forms.TabPage tabAutoComplete;
        private System.Windows.Forms.CheckBox cbShowGutterMargin;
        private System.Windows.Forms.CheckBox cbShowEOLMarker;
        private System.Windows.Forms.CheckBox cbShowScopeIndicator;
        private System.Windows.Forms.CheckBox cbShowWhitespace;
        private System.Windows.Forms.CheckBox cbBracketMatching;
        private System.Windows.Forms.CheckBox cbEnableAutoComplete;
        private System.Windows.Forms.CheckBox cbShowLineNumbers;
        private System.Windows.Forms.CheckBox cbAutoCompleteSimpleExpressions;
        private System.Windows.Forms.CheckBox cbAutoCompleteComplexExpressions;
        private System.Windows.Forms.CheckBox cbAutoCompleteMatchOnBeginningOnly;
        private System.Windows.Forms.TabPage tabDebug;
        private System.Windows.Forms.CheckBox cbDebugShowConsole;
        private System.Windows.Forms.CheckBox cbEnableCodeFolding;
        private System.Windows.Forms.TabPage tabDefaultCode;
        private NTDLS.Syntax.SyntaxDocument syntaxDocument1;
        private NTDLS.Windows.Forms.CodeEditorControl _CodeEditor;
    }
}