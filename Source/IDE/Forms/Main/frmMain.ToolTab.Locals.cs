using System;
using System.Text;
using System.Windows.Forms;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        public delegate void deRemoveNonUpdatedLocals();
        public delegate void deUpdateLocalsValue(String sName, int iScope, String sType, String sValue);

        void InitializeLocalsGrid()
        {
            _LocalsGrid.MouseUp += _LocalsGrid_MouseUp;

            _LocalsGrid.Columns.Add("Name", "Name");
            _LocalsGrid.Columns.Add("Scope", "Scope");
            _LocalsGrid.Columns.Add("Type", "Type");
            _LocalsGrid.Columns.Add("Value", "Value");

            _LocalsGrid.Tag = 0; //used to keep track of the rows that have been updated.
        }

        private void _LocalsGrid_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                int rowIndex = _LocalsGrid.HitTest(e.X, e.Y).RowIndex;
                if (rowIndex >= 0)
                {
                    ContextMenuStrip popupMenu = new ContextMenuStrip();
                    popupMenu.ItemClicked += new ToolStripItemClickedEventHandler(_LocalsGrid_MouseUp_ItemClicked);

                    DataGridViewRow row = _LocalsGrid.Rows[rowIndex];

                    row.Selected = true;

                    popupMenu.Tag = row;

                    popupMenu.Items.Add("Copy");
                    popupMenu.Items.Add("Copy Value");
                    popupMenu.Items.Add("-");
                    popupMenu.Items.Add("Delete");
                    popupMenu.Items.Add("-");
                    popupMenu.Items.Add("Cancel");

                    popupMenu.Show(_LocalsGrid, e.Location);
                }
            }
        }

        void _LocalsGrid_MouseUp_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
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
                    Clipboard.SetText(clickedRow.Cells["Value"].Value.ToString());
                    break;
                case "Delete":
                    _LocalsGrid.Rows.Remove(clickedRow);
                    break;
                case "Cancel":
                    break;
            }
        }

        void UpdateAllLocalVariables()
        {
            WriteToCmdPipe("::ListLocalVariables~");
            _LocalsGrid.Tag = ((int)_LocalsGrid.Tag) + 1;
        }

        void RemoveNonUpdatedLocals()
        {
            for (int iRow = _LocalsGrid.Rows.Count - 1; iRow > -1; iRow--)
            {
                if (_LocalsGrid.Rows[iRow].Tag != _LocalsGrid.Tag)
                {
                    _LocalsGrid.Rows.Remove(_LocalsGrid.Rows[iRow]);
                }
            }

            _LocalsGrid.AutoResizeColumns(DataGridViewAutoSizeColumnsMode.AllCells);
        }

        public void UpdateLocalsValue(String sName, int iScope, String sType, String sValue)
        {
            bool bFound = false;

            if (_TabTools.SelectedTab != tabLocals)
            {
                if (_WatchGrid.Rows.Count == 0)
                {
                    _TabTools.SelectedTab = tabLocals;
                }
            }

            foreach (DataGridViewRow row in _LocalsGrid.Rows)
            {
                if (row.Cells["Name"].Value.ToString().ToUpper() == sName.ToUpper())
                {
                    row.Cells["Scope"].Value = iScope;
                    row.Cells["Type"].Value = sType;
                    row.Cells["Value"].Value = sValue;
                    row.Tag = _LocalsGrid.Tag;
                    bFound = true;
                }
            }

            if (!bFound)
            {
                AddLocalsValue(sName);
                UpdateLocalsValue(sName, iScope, sType, sValue);
                return;
            }
        }

        void AddLocalsValue(string sName)
        {
            if (_TabTools.SelectedTab != tabLocals)
            {
                _TabTools.SelectedTab = tabLocals;
            }

            DataGridViewRow row = new DataGridViewRow();

            DataGridViewTextBoxCell nameCell = new DataGridViewTextBoxCell();
            nameCell.Value = sName;
            row.Cells.Add(nameCell);

            DataGridViewTextBoxCell typeCell = new DataGridViewTextBoxCell();
            typeCell.Value = "<unknown>";
            row.Cells.Add(typeCell);

            DataGridViewTextBoxCell valueCell = new DataGridViewTextBoxCell();
            valueCell.Value = "<unknown>";
            row.Cells.Add(valueCell);

            row.Tag = _LocalsGrid.Tag;

            _LocalsGrid.Rows.Add(row);
        }
    }
}
