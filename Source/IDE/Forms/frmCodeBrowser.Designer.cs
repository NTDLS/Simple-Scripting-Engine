namespace SSIDE.Forms
{
    partial class frmCodeBrowser
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frmCodeBrowser));
            this._SyntaxDocument = new NTDLS.Syntax.SyntaxDocument(this.components);
            this.panelHeader = new System.Windows.Forms.Panel();
            this.cmdBrowse = new System.Windows.Forms.Button();
            this.lblDescriptionValue = new System.Windows.Forms.Label();
            this.lblDescription = new System.Windows.Forms.Label();
            this.lblLocation = new System.Windows.Forms.Label();
            this._LibraryLocation = new System.Windows.Forms.TextBox();
            this._SplitContainer = new System.Windows.Forms.SplitContainer();
            this._FileList = new System.Windows.Forms.ListBox();
            this.lblDoubleClick = new System.Windows.Forms.Label();
            this._CodeEditor = new NTDLS.Windows.Forms.CodeEditorControl();
            this.lblFiller = new System.Windows.Forms.Label();
            this.panelHeader.SuspendLayout();
            this._SplitContainer.Panel1.SuspendLayout();
            this._SplitContainer.Panel2.SuspendLayout();
            this._SplitContainer.SuspendLayout();
            this.SuspendLayout();
            // 
            // _SyntaxDocument
            // 
            this._SyntaxDocument.Lines = new string[] {
        ""};
            this._SyntaxDocument.MaxUndoBufferSize = 1000;
            this._SyntaxDocument.Modified = false;
            this._SyntaxDocument.UndoStep = 0;
            // 
            // panelHeader
            // 
            this.panelHeader.Controls.Add(this.cmdBrowse);
            this.panelHeader.Controls.Add(this.lblDescriptionValue);
            this.panelHeader.Controls.Add(this.lblDescription);
            this.panelHeader.Controls.Add(this.lblLocation);
            this.panelHeader.Controls.Add(this._LibraryLocation);
            this.panelHeader.Dock = System.Windows.Forms.DockStyle.Top;
            this.panelHeader.Location = new System.Drawing.Point(3, 3);
            this.panelHeader.Name = "panelHeader";
            this.panelHeader.Size = new System.Drawing.Size(626, 73);
            this.panelHeader.TabIndex = 6;
            // 
            // cmdBrowse
            // 
            this.cmdBrowse.Location = new System.Drawing.Point(588, 1);
            this.cmdBrowse.Name = "cmdBrowse";
            this.cmdBrowse.Size = new System.Drawing.Size(28, 23);
            this.cmdBrowse.TabIndex = 8;
            this.cmdBrowse.Text = "...";
            this.cmdBrowse.UseVisualStyleBackColor = true;
            this.cmdBrowse.Click += new System.EventHandler(this.cmdBrowse_Click);
            // 
            // lblDescriptionValue
            // 
            this.lblDescriptionValue.Location = new System.Drawing.Point(66, 31);
            this.lblDescriptionValue.Name = "lblDescriptionValue";
            this.lblDescriptionValue.Size = new System.Drawing.Size(550, 39);
            this.lblDescriptionValue.TabIndex = 7;
            this.lblDescriptionValue.Text = "...";
            // 
            // lblDescription
            // 
            this.lblDescription.AutoSize = true;
            this.lblDescription.Location = new System.Drawing.Point(3, 31);
            this.lblDescription.Name = "lblDescription";
            this.lblDescription.Size = new System.Drawing.Size(63, 13);
            this.lblDescription.TabIndex = 6;
            this.lblDescription.Text = "Description:";
            // 
            // lblLocation
            // 
            this.lblLocation.AutoSize = true;
            this.lblLocation.Location = new System.Drawing.Point(12, 6);
            this.lblLocation.Name = "lblLocation";
            this.lblLocation.Size = new System.Drawing.Size(51, 13);
            this.lblLocation.TabIndex = 5;
            this.lblLocation.Text = "Location:";
            // 
            // _LibraryLocation
            // 
            this._LibraryLocation.Location = new System.Drawing.Point(69, 3);
            this._LibraryLocation.Name = "_LibraryLocation";
            this._LibraryLocation.ReadOnly = true;
            this._LibraryLocation.Size = new System.Drawing.Size(513, 20);
            this._LibraryLocation.TabIndex = 4;
            // 
            // _SplitContainer
            // 
            this._SplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this._SplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this._SplitContainer.Location = new System.Drawing.Point(3, 76);
            this._SplitContainer.Name = "_SplitContainer";
            // 
            // _SplitContainer.Panel1
            // 
            this._SplitContainer.Panel1.Controls.Add(this._FileList);
            this._SplitContainer.Panel1.Controls.Add(this.lblDoubleClick);
            // 
            // _SplitContainer.Panel2
            // 
            this._SplitContainer.Panel2.Controls.Add(this._CodeEditor);
            this._SplitContainer.Panel2.Controls.Add(this.lblFiller);
            this._SplitContainer.Size = new System.Drawing.Size(626, 394);
            this._SplitContainer.SplitterDistance = 240;
            this._SplitContainer.TabIndex = 7;
            // 
            // _FileList
            // 
            this._FileList.Dock = System.Windows.Forms.DockStyle.Fill;
            this._FileList.FormattingEnabled = true;
            this._FileList.Location = new System.Drawing.Point(0, 13);
            this._FileList.Name = "_FileList";
            this._FileList.Size = new System.Drawing.Size(240, 381);
            this._FileList.Sorted = true;
            this._FileList.TabIndex = 2;
            this._FileList.SelectedIndexChanged += new System.EventHandler(this._FileList_SelectedIndexChanged);
            this._FileList.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this._FileList_MouseDoubleClick);
            // 
            // lblDoubleClick
            // 
            this.lblDoubleClick.AutoSize = true;
            this.lblDoubleClick.Dock = System.Windows.Forms.DockStyle.Top;
            this.lblDoubleClick.Location = new System.Drawing.Point(0, 0);
            this.lblDoubleClick.Name = "lblDoubleClick";
            this.lblDoubleClick.Size = new System.Drawing.Size(112, 13);
            this.lblDoubleClick.TabIndex = 1;
            this.lblDoubleClick.Text = "Double click to select:";
            // 
            // _CodeEditor
            // 
            this._CodeEditor.ActiveView = NTDLS.Windows.Forms.CodeEditor.ActiveView.BottomRight;
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
            this._CodeEditor.Location = new System.Drawing.Point(0, 13);
            this._CodeEditor.LockCursorUpdate = false;
            this._CodeEditor.Name = "_CodeEditor";
            this._CodeEditor.Saved = false;
            this._CodeEditor.ShowScopeIndicator = false;
            this._CodeEditor.Size = new System.Drawing.Size(382, 381);
            this._CodeEditor.SmoothScroll = false;
            this._CodeEditor.SplitviewH = -4;
            this._CodeEditor.SplitviewV = -4;
            this._CodeEditor.TabGuideColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(219)))), ((int)(((byte)(214)))));
            this._CodeEditor.TabIndex = 4;
            this._CodeEditor.WhitespaceColor = System.Drawing.SystemColors.ControlDark;
            // 
            // lblFiller
            // 
            this.lblFiller.AutoSize = true;
            this.lblFiller.Dock = System.Windows.Forms.DockStyle.Top;
            this.lblFiller.Location = new System.Drawing.Point(0, 0);
            this.lblFiller.Name = "lblFiller";
            this.lblFiller.Size = new System.Drawing.Size(67, 13);
            this.lblFiller.TabIndex = 2;
            this.lblFiller.Text = "                    ";
            // 
            // frmCodeBrowser
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(632, 473);
            this.Controls.Add(this._SplitContainer);
            this.Controls.Add(this.panelHeader);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(640, 230);
            this.Name = "frmCodeBrowser";
            this.Padding = new System.Windows.Forms.Padding(3);
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "CodeBrowser";
            this.Load += new System.EventHandler(this.frmCodeBrowser_Load);
            this.panelHeader.ResumeLayout(false);
            this.panelHeader.PerformLayout();
            this._SplitContainer.Panel1.ResumeLayout(false);
            this._SplitContainer.Panel1.PerformLayout();
            this._SplitContainer.Panel2.ResumeLayout(false);
            this._SplitContainer.Panel2.PerformLayout();
            this._SplitContainer.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private NTDLS.Syntax.SyntaxDocument _SyntaxDocument;
        private System.Windows.Forms.Panel panelHeader;
        private System.Windows.Forms.TextBox _LibraryLocation;
        private System.Windows.Forms.SplitContainer _SplitContainer;
        private System.Windows.Forms.ListBox _FileList;
        private System.Windows.Forms.Label lblDoubleClick;
        private NTDLS.Windows.Forms.CodeEditorControl _CodeEditor;
        private System.Windows.Forms.Label lblFiller;
        private System.Windows.Forms.Button cmdBrowse;
        private System.Windows.Forms.Label lblDescriptionValue;
        private System.Windows.Forms.Label lblDescription;
        private System.Windows.Forms.Label lblLocation;
    }
}