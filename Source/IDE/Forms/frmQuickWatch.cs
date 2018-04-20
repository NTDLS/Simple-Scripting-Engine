using System;
using System.Windows.Forms;

namespace SSIDE.Forms
{
    public partial class frmQuickWatch : Form
    {
        public frmQuickWatch()
        {
            InitializeComponent();
        }

        public string PropertyName
        {
            get { return textBoxPropertyName.Text.Trim(); }
            set { textBoxPropertyName.Text = value.ToString(); }
        }

        public string PropertyType
        {
            get { return textBoxPropertyType.Text.Trim(); }
            set { textBoxPropertyType.Text = value.ToString(); }
        }

        public string PropertyValue
        {
            get { return textBoxPropertyValue.Text.Trim(); }
            set { textBoxPropertyValue.Text = value.ToString(); }
        }

        private void frmQuickWatch_Load(object sender, EventArgs e)
        {
            this.CancelButton = buttonClose;
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
