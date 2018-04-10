using System;
using System.Text;
using System.Windows.Forms;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        public delegate void deUpdateWatchValue(String sExpression, String sType, String sValue);

        enum WatchRowType
        {
            UserAdded,
            SystemAdded
        }

        void InitializeWatchGrid()
        {
            _WatchGrid.Columns.Add("Name", "Name");
            _WatchGrid.Columns.Add("Type", "Type");
            _WatchGrid.Columns.Add("Value", "Value");

            _WatchGrid.CellMouseDoubleClick += _WatchGrid_CellMouseDoubleClick;
            _WatchGrid.MouseUp += _WatchGrid_MouseUp;
        }

        void AddWatch(WatchRowType rowType, string expressionText, bool requestUpdate)
        {
            DataGridViewRow row = new DataGridViewRow();

            row.Tag = rowType;

            DataGridViewTextBoxCell nameCell = new DataGridViewTextBoxCell();
            nameCell.Value = expressionText;
            row.Cells.Add(nameCell);

            DataGridViewTextBoxCell typeCell = new DataGridViewTextBoxCell();
            typeCell.Value = "<unknown>";
            row.Cells.Add(typeCell);

            DataGridViewTextBoxCell valueCell = new DataGridViewTextBoxCell();
            valueCell.Value = "<unknown>";
            row.Cells.Add(valueCell);

            _WatchGrid.Rows.Add(row);

            _TabTools.SelectedTab = tabWatch;

            if (requestUpdate)
            {
                UpdateWatchValue(expressionText);
            }
        }

        void AddWatch(WatchRowType rowType, string expressionText)
        {
            AddWatch(rowType, expressionText, true);
        }

        private void _WatchGrid_CellMouseDoubleClick(object sender, DataGridViewCellMouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left && e.RowIndex >= 0)
            {
                _WatchForm.ExpressionText = _WatchGrid.Rows[e.RowIndex].Cells["Name"].Value.ToString();
                if (_RunningApplication.IsRunning)
                {
                    WriteToCmdPipe("::ListVariables~");
                    _WatchForm.Enabled = false;
                    while (!_WatchForm.Enabled)
                    {
                        System.Threading.Thread.Sleep(100);
                    }
                }
                else
                {
                    _WatchForm.Enabled = true;
                }

                if (_WatchForm.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    _WatchGrid.Rows[e.RowIndex].Cells["Name"].Value = _WatchForm.ExpressionText;
                    _WatchGrid.Rows[e.RowIndex].Cells["Type"].Value = "<unknown>";
                    _WatchGrid.Rows[e.RowIndex].Cells["Value"].Value = "<unknown>";
                    _WatchGrid.Rows[e.RowIndex].Tag = WatchRowType.UserAdded;

                    UpdateWatchValue(_WatchForm.ExpressionText);
                }
            }
        }

        private void _WatchGrid_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                ContextMenuStrip popupMenu = new ContextMenuStrip();
                popupMenu.ItemClicked += new ToolStripItemClickedEventHandler(_WatchGrid_MouseUp_ItemClicked);

                DataGridViewRow row = null;

                int rowIndex = _WatchGrid.HitTest(e.X, e.Y).RowIndex;
                if (rowIndex >= 0)
                {
                    row = _WatchGrid.Rows[rowIndex];
                    row.Selected = true;
                    popupMenu.Tag = row;
                }
                else
                {
                    popupMenu.Tag = null;
                }

                popupMenu.Items.Add("Add");
                if (row != null)
                {
                    if (((WatchRowType)row.Tag) == WatchRowType.UserAdded)
                    {
                        popupMenu.Items.Add("Edit");
                    }
                    popupMenu.Items.Add("Copy");
                    popupMenu.Items.Add("Copy Value");
                    popupMenu.Items.Add("-");
                    popupMenu.Items.Add("Delete");
                }
                if (_WatchGrid.Rows.Count > 0)
                {
                    popupMenu.Items.Add("Delete All");
                    popupMenu.Items.Add("-");
                }
                if (row != null && _RunningApplication.IsRunning)
                {
                    popupMenu.Items.Add("Refresh");
                    popupMenu.Items.Add("-");
                }
                popupMenu.Items.Add("Cancel");

                popupMenu.Show(_WatchGrid, e.Location);
            }
        }

        void _WatchGrid_MouseUp_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            ContextMenuStrip senderMenu = (ContextMenuStrip)sender;
            DataGridViewRow clickedRow = null;

            senderMenu.Close();

            if (senderMenu.Tag != null)
            {
                clickedRow = (DataGridViewRow)senderMenu.Tag;
            }

            if (e.ClickedItem.Text == "Copy")
            {
                System.Text.StringBuilder text = new StringBuilder();
                foreach (DataGridViewCell cell in clickedRow.Cells)
                {
                    text.Append(cell.Value.ToString() + "\t");
                }
                text.Length--; //Remove trailing [tab].
                Clipboard.SetText(text.ToString());
            }
            if (e.ClickedItem.Text == "Copy Value")
            {
                Clipboard.SetText(clickedRow.Cells["Value"].Value.ToString());
            }
            else if (e.ClickedItem.Text == "Add")
            {
                if (_RunningApplication.IsRunning)
                {
                    WriteToCmdPipe("::ListVariables~");
                    _WatchForm.Enabled = false;
                    while (!_WatchForm.Enabled)
                    {
                        System.Threading.Thread.Sleep(100);
                    }
                }
                else
                {
                    _WatchForm.Enabled = true;
                }

                if (_WatchForm.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    AddWatch(WatchRowType.UserAdded, _WatchForm.ExpressionText);
                }
            }
            else if (e.ClickedItem.Text == "Edit")
            {
                if (((WatchRowType)clickedRow.Tag) == WatchRowType.UserAdded)
                {
                    _WatchForm.ExpressionText = clickedRow.Cells["Name"].Value.ToString();

                    if (_RunningApplication.IsRunning)
                    {
                        WriteToCmdPipe("::ListVariables~");
                        _WatchForm.Enabled = false;
                        while (!_WatchForm.Enabled)
                        {
                            System.Threading.Thread.Sleep(100);
                        }
                    }
                    else
                    {
                        _WatchForm.Enabled = true;
                    }

                    if (_WatchForm.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                    {
                        clickedRow.Cells["Name"].Value = _WatchForm.ExpressionText;
                        clickedRow.Cells["Type"].Value = "<unknown>";
                        clickedRow.Cells["Value"].Value = "<unknown>";
                        clickedRow.Tag = WatchRowType.UserAdded;
                        UpdateWatchValue(_WatchForm.ExpressionText);
                    }
                }
            }
            else if (e.ClickedItem.Text == "Delete")
            {
                _WatchGrid.Rows.Remove(clickedRow);
            }
            else if (e.ClickedItem.Text == "Delete All")
            {
                DialogResult result = MessageBox.Show("Are you sure you want to delete all watch values?",
                    Application.ProductName, MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2);

                if (result == System.Windows.Forms.DialogResult.Yes)
                {
                    DeleteAllWatchValues();
                }
            }
            else if (e.ClickedItem.Text == "Refresh")
            {
                UpdateWatchValue(clickedRow.Cells["Name"].Value.ToString());
            }
        }

        void UpdateWatchValue(string expressionText)
        {
            if (_RunningApplication.IsRunning)
            {
                WriteToCmdPipe("::SymbolInfo~|" + expressionText.Trim().ToString());
            }
        }

        void UpdateAllWatchValues()
        {
            if (_RunningApplication.IsRunning)
            {
                foreach (DataGridViewRow row in _WatchGrid.Rows)
                {
                    if (((WatchRowType)row.Tag) == WatchRowType.UserAdded)
                    {
                        UpdateWatchValue(row.Cells["Name"].Value.ToString());
                    }
                }
            }
        }

        void DeleteAllSystemAddedWatchValues()
        {
            for (int i = _WatchGrid.Rows.Count - 1; i != -1; i--)
            {
                if (((WatchRowType)_WatchGrid.Rows[i].Tag) == WatchRowType.SystemAdded)
                {
                    _WatchGrid.Rows.Remove(_WatchGrid.Rows[i]);
                }
            }
        }

        void DeleteAllWatchValues()
        {
            for (int i = _WatchGrid.Rows.Count - 1; i != -1; i--)
            {
                _WatchGrid.Rows.Remove(_WatchGrid.Rows[i]);
            }
        }

        public void UpdateWatchValue(String sExpression, String sType, String sValue)
        {
            bool bFound = false;
            if (_TabTools.SelectedTab != tabWatch)
            {
                _TabTools.SelectedTab = tabWatch;
            }

            foreach (DataGridViewRow row in _WatchGrid.Rows)
            {
                if (row.Cells["Name"].Value.ToString().ToUpper() == sExpression.ToUpper())
                {
                    row.Cells["Type"].Value = sType;
                    row.Cells["Value"].Value = sValue;
                    bFound = true;
                }
            }

            if (!bFound)
            {
                AddWatch(WatchRowType.SystemAdded, sExpression, false);
                UpdateWatchValue(sExpression, sType, sValue);
                return;
            }

            if (sType == "<dynamic>")
            {
                _WatchGrid.AutoResizeColumns(DataGridViewAutoSizeColumnsMode.AllCells);
            }
        }
    }
}
