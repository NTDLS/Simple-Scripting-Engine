using System;
using System.Windows.Forms;
using SSIDE.Classes;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        public delegate void deClearFilesGrid();
        public delegate void deAutosizeFileGrid();
        public delegate void deAddFileToGrid(String sName);

        void InitializeFilesGrid()
        {
            _FilesGrid.CellDoubleClick += new DataGridViewCellEventHandler(_FilesGrid_CellDoubleClick);

            _FilesGrid.Columns.Add("Name", "Name");
            _FilesGrid.Columns["Name"].Width = 300;
        }

        void _FilesGrid_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex >= 0)
            {
                try
                {
                    string fileName = _FilesGrid.Rows[e.RowIndex].Cells["Name"].Value.ToString();
                    DocTabPage tab = GetTabByFilename(fileName, true);
                    if (tab != null)
                    {
                        tab.CodeEditor.Focus();
                        _TabDocs.SelectedTab = tab;
                    }
                }
                catch
                {
                }
            }
        }

        public void ClearFilesGrid()
        {
            _FilesGrid.Rows.Clear();
        }

        public void AutosizeFileGrid()
        {
            _FilesGrid.AutoResizeColumns(DataGridViewAutoSizeColumnsMode.AllCells);
        }

        public void AddFileToGrid(String sName)
        {
            DataGridViewRow row = new DataGridViewRow();

            DataGridViewTextBoxCell nameCell = new DataGridViewTextBoxCell();
            nameCell.Value = sName;
            row.Cells.Add(nameCell);

            _FilesGrid.Rows.Add(row);
        }

        void UpdateAllLoadedFiles()
        {
            WriteToCmdPipe("::BeginFileList~");
            _LocalsGrid.Tag = ((int)_LocalsGrid.Tag) + 1;
        }

    }
}
