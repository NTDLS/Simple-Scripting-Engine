using System;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using SSIDE.Classes;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        public delegate void deAddErrorToList(String sText);

        void InitializeErrorsGrid()
        {
            _ErrorGrid.CellDoubleClick += _ErrorGrid_CellDoubleClick;
            _ErrorGrid.MouseUp += _ErrorGrid_MouseUp;

            _ErrorGrid.Columns.Add("Sequence", "Seq.");
            _ErrorGrid.Columns.Add("File", "File");
            _ErrorGrid.Columns.Add("Line", "Line");
            _ErrorGrid.Columns.Add("Message", "Message");
        }

        public void AddErrorToList(String sText)
        {
            DataGridViewRow row = new DataGridViewRow();

            string[] fields = sText.Trim().Split('\t');

            if (fields.Length >= 4)
            {
                DataGridViewTextBoxCell sequenceCell = new DataGridViewTextBoxCell();
                sequenceCell.Value = fields[0];
                row.Cells.Add(sequenceCell);

                DataGridViewTextBoxCell fileCell = new DataGridViewTextBoxCell();
                fileCell.Value = fields[1];
                row.Cells.Add(fileCell);

                DataGridViewTextBoxCell lineCell = new DataGridViewTextBoxCell();
                lineCell.Value = fields[2];
                row.Cells.Add(lineCell);

                DataGridViewTextBoxCell errorCell = new DataGridViewTextBoxCell();
                errorCell.Value = fields[3];
                row.Cells.Add(errorCell);

                _ErrorGrid.Rows.Add(row);
            }
        }

        private void _ErrorGrid_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex >= 0)
            {
                int fileLine = int.Parse(_ErrorGrid.Rows[e.RowIndex].Cells["Line"].Value.ToString());

                try
                {
                    string fileName = _ErrorGrid.Rows[e.RowIndex].Cells["File"].Value.ToString();

                    DocTabPage tab = GetTabByFilename(fileName, true);
                    if (tab != null)
                    {
                        tab.CodeEditor.GotoLine(fileLine - 1);
                        tab.CodeEditor.HighLightActiveLine = true;
                        tab.CodeEditor.HighLightedLineColor = Color.FromArgb(255, 230, 230);
                        tab.CodeEditor.Focus();

                        _TabDocs.SelectedTab = tab;
                    }
                }
                catch
                {
                }
            }
        }

        private void _ErrorGrid_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                int rowIndex = _ErrorGrid.HitTest(e.X, e.Y).RowIndex;
                if (rowIndex >= 0)
                {
                    ContextMenuStrip popupMenu = new ContextMenuStrip();
                    popupMenu.ItemClicked += new ToolStripItemClickedEventHandler(_ErrorGrid_MouseUp_ItemClicked);

                    DataGridViewRow row = _ErrorGrid.Rows[rowIndex];

                    row.Selected = true;

                    popupMenu.Tag = row;

                    popupMenu.Items.Add("Copy");
                    popupMenu.Items.Add("Copy Value");
                    popupMenu.Items.Add("-");
                    popupMenu.Items.Add("Delete");
                    popupMenu.Items.Add("-");
                    popupMenu.Items.Add("Cancel");

                    popupMenu.Show(_ErrorGrid, e.Location);
                }
            }
        }

        void _ErrorGrid_MouseUp_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            ContextMenuStrip senderMenu = (ContextMenuStrip)sender;
            DataGridViewRow clickedRow = (DataGridViewRow)senderMenu.Tag;

            senderMenu.Close();

            switch (e.ClickedItem.Text)
            {
                case "Copy":
                    System.Text.StringBuilder text = new StringBuilder();
                    foreach (DataGridViewCell cell in clickedRow.Cells)
                    {
                        text.Append(cell.Value.ToString() + "\t");
                    }
                    text.Length--; //Remove trailing [tab].
                    Clipboard.SetText(text.ToString());
                    break;
                case "Copy Value":
                    Clipboard.SetText(clickedRow.Cells["Message"].Value.ToString());
                    break;
                case "Delete":
                    _ErrorGrid.Rows.Remove(clickedRow);
                    break;
                case "Cancel":
                    break;
            }
        }
    }
}
