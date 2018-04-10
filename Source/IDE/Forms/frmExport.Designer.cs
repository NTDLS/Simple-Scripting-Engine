namespace SSIDE.Forms
{
    partial class frmExport
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frmExport));
            this.label1 = new System.Windows.Forms.Label();
            this.txtExportLocation = new System.Windows.Forms.TextBox();
            this.txtRelativeImageURI = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.cmdSelectLocation = new System.Windows.Forms.Button();
            this.cmdExport = new System.Windows.Forms.Button();
            this.cmdCancel = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(16, 26);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(82, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Export Filename";
            // 
            // txtExportLocation
            // 
            this.txtExportLocation.Location = new System.Drawing.Point(19, 42);
            this.txtExportLocation.Name = "txtExportLocation";
            this.txtExportLocation.Size = new System.Drawing.Size(286, 20);
            this.txtExportLocation.TabIndex = 1;
            // 
            // txtRelativeImageURI
            // 
            this.txtRelativeImageURI.Location = new System.Drawing.Point(19, 81);
            this.txtRelativeImageURI.Name = "txtRelativeImageURI";
            this.txtRelativeImageURI.Size = new System.Drawing.Size(317, 20);
            this.txtRelativeImageURI.TabIndex = 4;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(16, 65);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(100, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Relative Image URI";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.cmdSelectLocation);
            this.groupBox1.Controls.Add(this.txtExportLocation);
            this.groupBox1.Controls.Add(this.txtRelativeImageURI);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(353, 121);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Export to HTML";
            // 
            // cmdSelectLocation
            // 
            this.cmdSelectLocation.Location = new System.Drawing.Point(311, 42);
            this.cmdSelectLocation.Name = "cmdSelectLocation";
            this.cmdSelectLocation.Size = new System.Drawing.Size(25, 20);
            this.cmdSelectLocation.TabIndex = 2;
            this.cmdSelectLocation.Text = "...";
            this.cmdSelectLocation.UseVisualStyleBackColor = true;
            this.cmdSelectLocation.Click += new System.EventHandler(this.cmdSelectLocation_Click);
            // 
            // cmdExport
            // 
            this.cmdExport.Location = new System.Drawing.Point(209, 139);
            this.cmdExport.Name = "cmdExport";
            this.cmdExport.Size = new System.Drawing.Size(75, 23);
            this.cmdExport.TabIndex = 1;
            this.cmdExport.Text = "Export";
            this.cmdExport.UseVisualStyleBackColor = true;
            this.cmdExport.Click += new System.EventHandler(this.cmdExport_Click);
            // 
            // cmdCancel
            // 
            this.cmdCancel.Location = new System.Drawing.Point(290, 139);
            this.cmdCancel.Name = "cmdCancel";
            this.cmdCancel.Size = new System.Drawing.Size(75, 23);
            this.cmdCancel.TabIndex = 2;
            this.cmdCancel.Text = "Cancel";
            this.cmdCancel.UseVisualStyleBackColor = true;
            this.cmdCancel.Click += new System.EventHandler(this.cmdCancel_Click);
            // 
            // frmExport
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(377, 172);
            this.Controls.Add(this.cmdCancel);
            this.Controls.Add(this.cmdExport);
            this.Controls.Add(this.groupBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "frmExport";
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Export";
            this.Load += new System.EventHandler(this.frmExport_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txtExportLocation;
        private System.Windows.Forms.TextBox txtRelativeImageURI;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button cmdExport;
        private System.Windows.Forms.Button cmdCancel;
        private System.Windows.Forms.Button cmdSelectLocation;
    }
}