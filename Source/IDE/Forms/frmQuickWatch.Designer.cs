namespace SSIDE.Forms
{
    partial class frmQuickWatch
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frmQuickWatch));
            this.labelPropertyName = new System.Windows.Forms.Label();
            this.textBoxPropertyName = new System.Windows.Forms.TextBox();
            this.textBoxPropertyType = new System.Windows.Forms.TextBox();
            this.labelPropertyType = new System.Windows.Forms.Label();
            this.textBoxPropertyValue = new System.Windows.Forms.TextBox();
            this.labelPropertyValue = new System.Windows.Forms.Label();
            this.buttonClose = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // labelPropertyName
            // 
            this.labelPropertyName.AutoSize = true;
            this.labelPropertyName.Location = new System.Drawing.Point(12, 9);
            this.labelPropertyName.Name = "labelPropertyName";
            this.labelPropertyName.Size = new System.Drawing.Size(35, 13);
            this.labelPropertyName.TabIndex = 0;
            this.labelPropertyName.Text = "Name";
            // 
            // textBoxPropertyName
            // 
            this.textBoxPropertyName.Location = new System.Drawing.Point(15, 25);
            this.textBoxPropertyName.Name = "textBoxPropertyName";
            this.textBoxPropertyName.ReadOnly = true;
            this.textBoxPropertyName.Size = new System.Drawing.Size(355, 20);
            this.textBoxPropertyName.TabIndex = 1;
            // 
            // textBoxPropertyType
            // 
            this.textBoxPropertyType.Location = new System.Drawing.Point(15, 64);
            this.textBoxPropertyType.Name = "textBoxPropertyType";
            this.textBoxPropertyType.ReadOnly = true;
            this.textBoxPropertyType.Size = new System.Drawing.Size(355, 20);
            this.textBoxPropertyType.TabIndex = 3;
            // 
            // labelPropertyType
            // 
            this.labelPropertyType.AutoSize = true;
            this.labelPropertyType.Location = new System.Drawing.Point(12, 48);
            this.labelPropertyType.Name = "labelPropertyType";
            this.labelPropertyType.Size = new System.Drawing.Size(31, 13);
            this.labelPropertyType.TabIndex = 2;
            this.labelPropertyType.Text = "Type";
            // 
            // textBoxPropertyValue
            // 
            this.textBoxPropertyValue.AcceptsReturn = true;
            this.textBoxPropertyValue.AcceptsTab = true;
            this.textBoxPropertyValue.Location = new System.Drawing.Point(15, 103);
            this.textBoxPropertyValue.Multiline = true;
            this.textBoxPropertyValue.Name = "textBoxPropertyValue";
            this.textBoxPropertyValue.ReadOnly = true;
            this.textBoxPropertyValue.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxPropertyValue.Size = new System.Drawing.Size(588, 297);
            this.textBoxPropertyValue.TabIndex = 5;
            // 
            // labelPropertyValue
            // 
            this.labelPropertyValue.AutoSize = true;
            this.labelPropertyValue.Location = new System.Drawing.Point(12, 87);
            this.labelPropertyValue.Name = "labelPropertyValue";
            this.labelPropertyValue.Size = new System.Drawing.Size(34, 13);
            this.labelPropertyValue.TabIndex = 4;
            this.labelPropertyValue.Text = "Value";
            // 
            // buttonClose
            // 
            this.buttonClose.Location = new System.Drawing.Point(528, 406);
            this.buttonClose.Name = "buttonClose";
            this.buttonClose.Size = new System.Drawing.Size(75, 23);
            this.buttonClose.TabIndex = 6;
            this.buttonClose.Text = "Close";
            this.buttonClose.UseVisualStyleBackColor = true;
            this.buttonClose.Click += new System.EventHandler(this.buttonClose_Click);
            // 
            // frmQuickWatch
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(615, 436);
            this.Controls.Add(this.buttonClose);
            this.Controls.Add(this.textBoxPropertyValue);
            this.Controls.Add(this.labelPropertyValue);
            this.Controls.Add(this.textBoxPropertyType);
            this.Controls.Add(this.labelPropertyType);
            this.Controls.Add(this.textBoxPropertyName);
            this.Controls.Add(this.labelPropertyName);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "frmQuickWatch";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Watch Expression";
            this.Load += new System.EventHandler(this.frmQuickWatch_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelPropertyName;
        private System.Windows.Forms.TextBox textBoxPropertyName;
        private System.Windows.Forms.TextBox textBoxPropertyType;
        private System.Windows.Forms.Label labelPropertyType;
        private System.Windows.Forms.TextBox textBoxPropertyValue;
        private System.Windows.Forms.Label labelPropertyValue;
        private System.Windows.Forms.Button buttonClose;
    }
}