using System;
using System.Windows.Forms;

namespace SSIDE.Forms
{
    public partial class frmWatchExpression : Form
    {
        public frmWatchExpression()
        {
            InitializeComponent();
        }

        public string ExpressionText
        {
            get { return cboExpression.Text.Trim(); }
            set { cboExpression.Text = value.ToString(); }
        }

        private void frmWatchExpression_Load(object sender, EventArgs e)
        {
            this.CancelButton = cmdCancel;
            this.AcceptButton = cmdOk;
        }

        private void cmdCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Close();
        }

        private void cmdOk_Click(object sender, EventArgs e)
        {
            if (cboExpression.Text.Trim().Length > 0)
            {
                this.DialogResult = System.Windows.Forms.DialogResult.OK;
                this.Close();
            }
            else
            {
                MessageBox.Show("You must specify an expression.",
                    Application.ProductName, MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }
    }
}
