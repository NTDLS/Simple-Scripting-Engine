using System;
using System.Drawing;
using System.Windows.Forms;
using SSIDE.Classes;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        void CodeEditor_FindReplaceTextNotFound(object sender, string text)
        {
            MessageBox.Show("Search string '" + text + "' not found.");
        }

        void CodeEditor_MouseDown(object sender, MouseEventArgs e)
        {
            ((NTDLS.Windows.Forms.CodeEditorControl)sender).AutoListVisible = false;
        }

        class PopupMenuInfo
        {
            public NTDLS.Windows.Forms.CodeEditorControl Editor;
            public NTDLS.Syntax.Row Row;
            public string Text;

        }

        public delegate void deQuickWatchInfo(String sName, String sType, String sValue);

        public void QuickWatchInfo(String sName, String sType, String sValue)
        {
            using (QuickWatch quickWatch = new QuickWatch())
            {
                quickWatch.PropertyName = sName;
                quickWatch.PropertyType = sType;
                quickWatch.PropertyValue = sValue;

                quickWatch.ShowDialog();
            }
        }

        void CodeEditor_RowMouseUp(object sender, NTDLS.Windows.Forms.CodeEditor.RowMouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                ContextMenuStrip popupMenu = new ContextMenuStrip();
                popupMenu.ItemClicked += new ToolStripItemClickedEventHandler(CodeEditor_RowMouseUp_ItemClicked);

                PopupMenuInfo menuInfo = new PopupMenuInfo();

                menuInfo.Editor = (NTDLS.Windows.Forms.CodeEditorControl)sender;
                menuInfo.Row = menuInfo.Editor.Caret.CurrentRow;

                Point xE = menuInfo.Editor.PointToClient(Cursor.Position);

                if (menuInfo.Editor.Selection.Text.Trim().Length > 0)
                {
                    string WatchText = menuInfo.Editor.Selection.Text.Trim();
                    if (WatchText.Length > 0 && WatchText.Length < 1028)
                    {
                        if (WatchText.Length > 50)
                        {
                            WatchText = WatchText.Substring(0, 47) + "...";
                        }

                        popupMenu.Items.Add("Watch: " + WatchText, TransparentImage(Properties.Resources.TabWatch)).Tag = menuInfo.Editor.Selection.Text.Trim();
                        popupMenu.Items.Add("-");
                    }

                    if (_RunningApplication.IsRunning)
                    {
                        _RunningApplication.TipMenuItem = new ToolStripMenuItem();
                        _RunningApplication.TipMenuItem.Text = "Properties";
                        _RunningApplication.TipMenuItem.DropDownItems.Add("Name: " + menuInfo.Editor.Selection.Text.Trim());
                        _RunningApplication.TipMenuItem.DropDownOpening += new EventHandler(CodeEditor_RowMouseUp_DropDownOpening);
                        _RunningApplication.TipMenuItem.Tag = menuInfo.Editor.Selection.Text.Trim();

                        popupMenu.Items.Add(_RunningApplication.TipMenuItem);
                    }

                    if (popupMenu.Items.Count > 0)
                    {
                        popupMenu.Items.Add("Cut", TransparentImage(Properties.Resources.ToolCut)).Enabled = !_RunningApplication.IsRunning;
                        popupMenu.Items.Add("Copy", TransparentImage(Properties.Resources.ToolCopy));
                        popupMenu.Items.Add("Paste", TransparentImage(Properties.Resources.ToolPaste)).Enabled = !_RunningApplication.IsRunning;
                    }
                }
                else
                {
                    NTDLS.Syntax.TextPoint textPoint = menuInfo.Editor.CharFromPixel(xE.X, xE.Y);
                    if (textPoint != null)
                    {
                        //If nothing is selected, then move the cursor.
                        menuInfo.Editor.Caret.Position = textPoint;
                        menuInfo.Editor.Refresh();
                        menuInfo.Row = menuInfo.Editor.Caret.CurrentRow;

                        NTDLS.Syntax.Word word = menuInfo.Editor.Document.GetValidWordFromPos(textPoint);
                        if (word != null && word.Text.Trim().Length > 0)
                        {
                            string wordText = word.Text.Trim();
                            if (wordText.Substring(wordText.Length - 1) == ",")
                            {
                                wordText = wordText.Substring(0, wordText.Length - 1);
                            }
                            if (wordText.Trim().Length > 0)
                            {
                                string sFriendly = wordText.Trim().Replace("\r", " ").Replace("\n", " ").Replace("\t", " ");
                                if (sFriendly.Length > 50)
                                {
                                    sFriendly = sFriendly.Substring(0, 50) + "...";
                                }

                                menuInfo.Text = wordText;

                                string WatchText = wordText;
                                if (WatchText.Length > 0 && WatchText.Length < 1028)
                                {
                                    if (WatchText.Length > 50)
                                    {
                                        WatchText = WatchText.Substring(0, 47) + "...";
                                    }

                                    popupMenu.Items.Add("Watch: " + WatchText, TransparentImage(Properties.Resources.TabWatch)).Tag = wordText;
                                }

                                if (_RunningApplication.IsRunning)
                                {
                                    ToolStripMenuItem quickWatch = new ToolStripMenuItem();
                                    quickWatch.Text = "Quick Watch";

                                    quickWatch.Click += new EventHandler(CodeEditor_QuickWatch_RowMouseUp_DropDownOpening);

                                    quickWatch.Tag = wordText.Trim();

                                    popupMenu.Items.Add(quickWatch);

                                    if (popupMenu.Items.Count > 0)
                                    {
                                        popupMenu.Items.Add("-");
                                    }
                                }

                                if (_RunningApplication.IsRunning)
                                {
                                    _RunningApplication.TipMenuItem = new ToolStripMenuItem();
                                    _RunningApplication.TipMenuItem.Text = "Properties";

                                    _RunningApplication.TipMenuItem.DropDownItems.Add("Name: " + wordText.Trim());

                                    _RunningApplication.TipMenuItem.DropDownOpening += new EventHandler(CodeEditor_RowMouseUp_DropDownOpening);

                                    _RunningApplication.TipMenuItem.Tag = wordText.Trim();

                                    popupMenu.Items.Add(_RunningApplication.TipMenuItem);
                                }

                                if (popupMenu.Items.Count > 0)
                                {
                                    popupMenu.Items.Add("-");
                                }

                                popupMenu.Items.Add("Copy", TransparentImage(Properties.Resources.ToolCopy));
                                popupMenu.Items.Add("Paste", TransparentImage(Properties.Resources.ToolPaste)).Enabled = !_RunningApplication.IsRunning;
                            }
                        }
                    }
                }

                if (popupMenu.Items.Count == 0 && menuInfo.Row != null)
                {
                    menuInfo.Text = menuInfo.Row.Text;
                    if (menuInfo.Row.Text.Length > 0)
                    {
                        popupMenu.Items.Add("Cut", TransparentImage(Properties.Resources.ToolCut)).Enabled = !_RunningApplication.IsRunning;
                        popupMenu.Items.Add("Copy", TransparentImage(Properties.Resources.ToolCopy));
                    }
                    popupMenu.Items.Add("Paste", TransparentImage(Properties.Resources.ToolPaste)).Enabled = !_RunningApplication.IsRunning;
                }

                if (popupMenu.Items.Count > 0)
                {
                    popupMenu.Items.Add("-");
                    popupMenu.Items.Add("Toggle Breakpoint", TransparentImage(Properties.Resources.ToolBreakpoint));
                    popupMenu.Tag = menuInfo;
                    popupMenu.Show(menuInfo.Editor, xE);
                }
            }
        }

        void CodeEditor_QuickWatch_RowMouseUp_DropDownOpening(object sender, EventArgs e)
        {
            WriteToCmdPipe("::QuickWatch~|" + ((ToolStripMenuItem)sender).Tag.ToString());
        }

        void CodeEditor_RowMouseUp_DropDownOpening(object sender, EventArgs e)
        {
            WriteToCmdPipe("::ToolTipSymbolInfo~|" + ((ToolStripMenuItem)sender).Tag.ToString());
        }

        void CodeEditor_RowMouseUp_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            if (e.ClickedItem.Text != "Properties")
            {
                ContextMenuStrip senderMenu = (ContextMenuStrip)sender;
                PopupMenuInfo menuInfo = (PopupMenuInfo)senderMenu.Tag;

                senderMenu.Close();

                if (e.ClickedItem.Text.ToUpper().StartsWith("Watch:".ToUpper()))
                {
                    AddWatch(WatchRowType.UserAdded, (string)e.ClickedItem.Tag);
                }
                else if (e.ClickedItem.Text.ToUpper() == "Copy".ToUpper())
                {
                    if (menuInfo.Text != null && menuInfo.Text.Length > 0)
                    {
                        Clipboard.SetText(menuInfo.Text);
                    }
                    else
                    {
                        menuInfo.Editor.Copy();
                    }
                }
                else if (e.ClickedItem.Text.ToUpper() == "Paste".ToUpper())
                {
                    menuInfo.Editor.Paste();
                }
                else if (e.ClickedItem.Text.ToUpper() == "Cut".ToUpper())
                {
                    menuInfo.Editor.Cut();
                }
                else if (e.ClickedItem.Text.ToUpper() == "Toggle Breakpoint".ToUpper())
                {
                    menuInfo.Row.Breakpoint = !menuInfo.Row.Breakpoint;
                }
            }
        }

        void Document_BreakPointAdded(object sender, NTDLS.Syntax.RowEventArgs e)
        {
            if (_RunningApplication.IsRunning)
            {
                WriteToCmdPipe("::BreakPoint~|" + ((DocTabPage)_TabDocs.SelectedTab).CodeEditor.FileName + "|" + (e.Row.Index + 1));
            }
        }

        void Document_BreakPointRemoved(object sender, NTDLS.Syntax.RowEventArgs e)
        {
            if (_RunningApplication.IsRunning)
            {
                WriteToCmdPipe("::DeleteBreakPoint~|" + ((DocTabPage)_TabDocs.SelectedTab).CodeEditor.FileName + "|" + (e.Row.Index + 1));
            }
        }
    }
}
