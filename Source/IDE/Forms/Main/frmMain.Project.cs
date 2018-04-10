using System;
using System.Windows.Forms;
using SSIDE.Classes;
using System.IO;
using System.Data;
using System.Xml;
using System.Drawing;
using Microsoft.VisualBasic;

namespace SSIDE.Forms
{
    public partial class frmMain
	{
        #region Events

        private void _ProjectTree_KeyUp(object sender, KeyEventArgs e)
        {
            if (!IsProjectOpen())
            {
                return;
            }

            if (e.KeyCode == Keys.F2)
            {
                if (_ProjectTree.SelectedNode != null)
                {
                    _ProjectTree.SelectedNode.BeginEdit();
                }
            }
        }

        void ProjectTreeView_MouseClick_PopupMenu_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            if (!IsProjectOpen() || e.ClickedItem.Tag == null)
            {
                return;
            }

            ProjectTreeNode node = ((ProjectTreeNode)e.ClickedItem.Tag);

            if (sender.GetType() == typeof(ContextMenuStrip))
            {
                ((ContextMenuStrip)sender).Close();
            }

            bool clickHandled = true;

            if (e.ClickedItem.Text == "Folder")
            {
                node.Expand();
                ProjectTreeNode newNode = node.CreateNewFolder();
                _ProjectTree.SelectedNode = newNode;
                newNode.BeginEdit();
            }
            else if (e.ClickedItem.Text == "Add Existing Folder")
            {
                FolderBrowserDialog addExisting = new FolderBrowserDialog();
                addExisting.RootFolder = Environment.SpecialFolder.Desktop;
                addExisting.SelectedPath = Path.GetDirectoryName (ProjectFileName);
                addExisting.ShowNewFolderButton = false;
                if (addExisting.ShowDialog() == DialogResult.OK)
                {
                    DirectoryInfo source = new DirectoryInfo(addExisting.SelectedPath);
                    DirectoryInfo target = new DirectoryInfo(Path.Combine(Path.GetDirectoryName(ProjectFileName), Path.GetFileName(addExisting.SelectedPath)));

                    /*
                    if (target.FullName.StartsWith(source.FullName) || source.FullName.StartsWith(target.FullName))
                    {
                        MessageBox.Show("Cannot add the parent of the currently selected folder.");
                        return;
                    }
                    */

                    ProjectTreeNode newNode = AddExstingDirectory(source, target, node, true);
                    if (newNode != null)
                    {
                        _ProjectTree.SelectedNode = newNode;
                        newNode.Expand();
                    }
                }
            }
            else if (e.ClickedItem.Text == "Code File")
            {
                if (node.Parent != null)
                {
                    node.Parent.Expand();
                }
                ProjectTreeNode newNode = node.CreateNewFile(global.CodeFileExtension);
                _ProjectTree.SelectedNode = newNode;

                newNode.BeginEdit();
            }
            else if (e.ClickedItem.Text == "HTML File")
            {
                if (node.Parent != null)
                {
                    node.Parent.Expand();
                }
                ProjectTreeNode newNode = node.CreateNewFile(".html");
                _ProjectTree.SelectedNode = newNode;

                newNode.BeginEdit();
            }
            else if (e.ClickedItem.Text == "Set as startup object")
            {
                node.IsStartupObject = !node.IsStartupObject;
            }
            else if (e.ClickedItem.Text == "Add Existing File")
            {
                System.Windows.Forms.OpenFileDialog ofdAddFile = new System.Windows.Forms.OpenFileDialog();

                ofdAddFile.Multiselect = true;
                ofdAddFile.ShowHelp = false;
                ofdAddFile.DefaultExt = global.CodeFileExtension;
                ofdAddFile.CheckFileExists = true;
                ofdAddFile.CheckPathExists = true;
                ofdAddFile.ValidateNames = true;
                ofdAddFile.Filter = "Simple Script|*" + global.CodeFileExtension + "|Text Files|*.txt|All Files|*.*";

                if (ofdAddFile.ShowDialog() == DialogResult.OK)
                {
                    foreach (String sourceFileName in ofdAddFile.FileNames)
                    {
                        try
                        {
                            ProjectTreeNode newNode = node.AddFile(sourceFileName);
                            string targetFileName = newNode.LogicalPath();

                            if (sourceFileName.ToLower() != targetFileName.ToLower())
                            {
                                System.IO.File.Copy(sourceFileName, targetFileName);
                            }
                        }
                        catch (Exception ex)
                        {
                            MessageBox.Show("Cannot add the file [" + sourceFileName + "] due to the following error:\r\n\r\n" + ex.Message);
                        }
                    }
                }
            }
            else if (e.ClickedItem.Text == "Explore Folder")
            {
                try
                {
                    System.Diagnostics.Process process = new System.Diagnostics.Process();
                    process.StartInfo.UseShellExecute = true;
                    process.StartInfo.FileName = "explorer";
                    process.StartInfo.Arguments = node.LogicalPath();
                    process.Start();
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Cannot explore to the folder due to the following error:\r\n\r\n" + ex.Message);
                }
            }
            else
            {
                clickHandled = false;
            }

            if (!clickHandled) //These operations affected some tabs to be closed.
            {
                clickHandled = true;

                if (!CloseAffectedProjectTabs(node))
                {
                    return;
                }

                if (e.ClickedItem.Text == "Rename")
                {
                    node.BeginEdit();
                }
                else if (e.ClickedItem.Text == "Remove")
                {
                    if (MessageBox.Show("Are you sure you want to remove the " + node.BasicNodeType.ToString() + " \"" + node.Text + "\" from the project?", "Confirm Delete",
                        MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2) != System.Windows.Forms.DialogResult.Yes)
                    {
                        return;
                    }

                    node.Remove();
                }
                else if (e.ClickedItem.Text == "Delete")
                {
                    if (MessageBox.Show("Are you sure you want to PERMANENTLY delete the " + node.BasicNodeType.ToString() + " \"" + node.Text + "\"?", "Confirm Delete",
                        MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2) != System.Windows.Forms.DialogResult.Yes)
                    {
                        return;
                    }

                    try
                    {
                        if (node.BasicNodeType == ProjectTreeNode.BasicNodeTypes.File)
                        {
                            System.IO.File.Delete(node.LogicalPath());
                            node.Remove();
                        }
                        else if (node.BasicNodeType == ProjectTreeNode.BasicNodeTypes.Folder)
                        {
                            Directory.Delete(node.LogicalPath(), true);
                            node.Remove();
                        }
                    }
                    catch(Exception ex)
                    {
                        MessageBox.Show("The object cannot be deleted due to the error below, try removing it instead.\r\n\r\n" + ex.Message);
                    }
                }
                else if (e.ClickedItem.Text == "Delete File")
                {
                }
                else
                {
                    clickHandled = false;
                }
            }

            SaveProjectFile();
        }

        private void _ProjectTree_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            if (!IsProjectOpen())
            {
                return;
            }

            _ProjectTree.SelectedNode = e.Node;

            if (e.Button == System.Windows.Forms.MouseButtons.Right)
            {
                ContextMenuStrip popupMenu = new ContextMenuStrip();
                popupMenu.ItemClicked += new ToolStripItemClickedEventHandler(ProjectTreeView_MouseClick_PopupMenu_ItemClicked);

                ProjectTreeNode.BasicNodeTypes basicNodeType = ((ProjectTreeNode)e.Node).BasicNodeType;
                ProjectTreeNode.NodeTypes nodeType = ((ProjectTreeNode)e.Node).NodeType;

                if (basicNodeType == ProjectTreeNode.BasicNodeTypes.Project || basicNodeType == ProjectTreeNode.BasicNodeTypes.Folder)
                {
                    ToolStripMenuItem addNewMenu = (ToolStripMenuItem)(popupMenu.Items.Add("Add New", TransparentImage(Properties.Resources.TreeCodeFile.ToBitmap())));
                    addNewMenu.DropDownItemClicked += new ToolStripItemClickedEventHandler(ProjectTreeView_MouseClick_PopupMenu_ItemClicked);

                    addNewMenu.DropDownItems.Add("Code File", TransparentImage(Properties.Resources.TreeCodeFile.ToBitmap())).Tag = e.Node;
                    addNewMenu.DropDownItems.Add("HTML File", TransparentImage(Properties.Resources.TreeHTMLFile.ToBitmap())).Tag = e.Node;
                    addNewMenu.DropDownItems.Add("Folder", TransparentImage(Properties.Resources.TreeFolderFile.ToBitmap())).Tag = e.Node;

                    popupMenu.Items.Add("Add Existing Folder", TransparentImage(Properties.Resources.TreeFolderFile.ToBitmap())).Tag = e.Node;
                    popupMenu.Items.Add("Add Existing File", TransparentImage(Properties.Resources.TreeUnknownFile.ToBitmap())).Tag = e.Node;
                    popupMenu.Items.Add("-");
                    popupMenu.Items.Add("Explore Folder", TransparentImage(Properties.Resources.TreeExplore.ToBitmap())).Tag = e.Node;

                }
                else if (basicNodeType == ProjectTreeNode.BasicNodeTypes.File)
                {
                    if (nodeType == ProjectTreeNode.NodeTypes.CodeFile || nodeType == ProjectTreeNode.NodeTypes.CodeFileDefault)
                    {
                        popupMenu.Items.Add("Set as startup object", TransparentImage(Properties.Resources.ToolRun)).Tag = e.Node;
                    }
                }
                else if (basicNodeType == ProjectTreeNode.BasicNodeTypes.Invalid)
                {
                    popupMenu.Items.Add("Remove", TransparentImage(Properties.Resources.ToolRemove)).Tag = e.Node;
                }

                if (basicNodeType != ProjectTreeNode.BasicNodeTypes.Invalid && basicNodeType != ProjectTreeNode.BasicNodeTypes.Project)
                {
                    if (popupMenu.Items.Count > 0)
                    {
                        popupMenu.Items.Add("-");
                    }
                    popupMenu.Items.Add("Rename").Tag = e.Node;
                    popupMenu.Items.Add("Remove", TransparentImage(Properties.Resources.ToolRemove)).Tag = e.Node;
                    popupMenu.Items.Add("Delete", TransparentImage(Properties.Resources.ToolDelete)).Tag = e.Node;
                }

                popupMenu.Show(_ProjectTree, e.Location);
            }
        }

        private void _ProjectTree_AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            if (!IsProjectOpen())
            {
                return;
            }

            try
            {
                if (e.Label == null) //Determine if lable has changed.
                {
                    e.CancelEdit = true;
                    return;
                }

                if (e.Label == null || e.Label.Length == 0)
                {
                    e.CancelEdit = true;
                    MessageBox.Show("The object cannot be empty.");
                    return;
                }

                ProjectTreeNode node = (ProjectTreeNode)e.Node;
                if (!CloseAffectedProjectTabs((ProjectTreeNode)e.Node))
                {
                    e.CancelEdit = true;
                    return;
                }

                ProjectTreeNode projectNode = GetProjectNode();
                string projectPath = Path.GetDirectoryName(ProjectFileName);
                string oldLineage = (projectPath + "\\" + node.FileLineage()).Replace("\\\\", "\\");
                //string nodeText = (e.Label == null || e.Label.Length == 0) ? e.Node.Text : e.Label;
                string nodeText = e.Label;

                if (!Path.HasExtension(nodeText))
                {
                    if (Path.HasExtension(e.Node.Text))
                    {
                        nodeText = nodeText + Path.GetExtension(e.Node.Text);
                    }
                }

                string newLineage = node.Parent == null ? null : (projectPath + "\\" + ((ProjectTreeNode)node.Parent).FileLineage() + "\\" + nodeText).Replace("\\\\", "\\");

                if (node.BasicNodeType == ProjectTreeNode.BasicNodeTypes.Folder)
                {
                    if (Directory.Exists(oldLineage))
                    {
                        Directory.Move(oldLineage, newLineage);
                    }
                    else
                    {
                        Directory.CreateDirectory(newLineage);
                    }
                }
                else if (node.BasicNodeType == ProjectTreeNode.BasicNodeTypes.File)
                {
                    if (System.IO.File.Exists(oldLineage))
                    {
                        System.IO.File.Move(oldLineage, newLineage);
                    }
                    node.NodeType = ProjectTreeNode.DetermineNodeType(nodeText);
                }

                e.CancelEdit = true; //Were going to change the text manually (below) cancel the automatic update.
                e.Node.Text = nodeText;
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
                e.CancelEdit = true;
            }

            SaveProjectFile();
        }

        private void _ProjectTree_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            if (!IsProjectOpen())
            {
                return;
            }

            ProjectTreeNode node = (ProjectTreeNode)e.Node;

            if (node.NodeType == ProjectTreeNode.NodeTypes.CodeFile
                || node.NodeType == ProjectTreeNode.NodeTypes.CodeFileDefault
                || node.NodeType == ProjectTreeNode.NodeTypes.BatchFile
                || node.NodeType == ProjectTreeNode.NodeTypes.JavaScript
                || node.NodeType == ProjectTreeNode.NodeTypes.StyleFile
                || node.NodeType == ProjectTreeNode.NodeTypes.TextFile
                || node.NodeType == ProjectTreeNode.NodeTypes.XMLFile
                || node.NodeType == ProjectTreeNode.NodeTypes.HTMLFile)
            {
                string fileLineage = (Path.GetDirectoryName(ProjectFileName) + "\\" + ((ProjectTreeNode)e.Node).FileLineage()).Replace("\\\\", "\\");
                if (System.IO.File.Exists(fileLineage))
                {
                    DocTabPage docTabPage = GetTabByFilename(fileLineage);
                    if (docTabPage != null)
                    {
                        docTabPage.CodeEditor.Focus();
                        _TabDocs.SelectedTab = docTabPage;
                    }
                    else
                    {
                        AddNewTab(fileLineage);
                    }
                }
                else
                {
                    MessageBox.Show("This file does not exist or cannot be found: \"" + fileLineage + "\".");
                }
            }
        }

        #endregion

        #region Drag and Drop

        private void _ProjectTree_ItemDrag(object sender, ItemDragEventArgs e)
        {
            ProjectTreeNode node = ((ProjectTreeNode)e.Item);
            if (node.NodeType != ProjectTreeNode.NodeTypes.Invalid && node.NodeType != ProjectTreeNode.NodeTypes.Project)
            {
                DoDragDrop(e.Item, DragDropEffects.Move);
            }
        }

        private void _ProjectTree_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.Move;
        }

        private void _ProjectTree_DragDrop(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent("SSIDE.Classes.ProjectTreeNode", false))
            {
                Point pt = ((TreeView)sender).PointToClient(new Point(e.X, e.Y));
                ProjectTreeNode destinationNode = (ProjectTreeNode)((TreeView)sender).GetNodeAt(pt);
                ProjectTreeNode sourceNode = (ProjectTreeNode)e.Data.GetData("SSIDE.Classes.ProjectTreeNode");

                int insertionIndex = -1;

                if (destinationNode.BasicNodeType == ProjectTreeNode.BasicNodeTypes.File)
                {
                    insertionIndex = destinationNode.Index;
                    destinationNode = (ProjectTreeNode)destinationNode.Parent;
                }

                if (destinationNode.TreeView == sourceNode.TreeView && sourceNode != destinationNode)
                {
                    if (!CloseAffectedProjectTabs(sourceNode))
                    {
                        return;
                    }

                    string sourceFile = sourceNode.LogicalPath();
                    ProjectTreeNode oldParentNode = (ProjectTreeNode)sourceNode.Parent;
                    sourceNode.Remove();

                    if (insertionIndex < 0)
                    {
                        if (sourceNode.BasicNodeType == ProjectTreeNode.BasicNodeTypes.File)
                        {
                            insertionIndex = destinationNode.Nodes.Count;
                        }
                    }
                    
                    destinationNode.Nodes.Insert(insertionIndex, sourceNode);
                    destinationNode.Expand();

                    string destinationFile = sourceNode.LogicalPath();

                    try
                    {
                        if (sourceFile.ToLower() != destinationFile.ToLower())
                        {
                            if (sourceNode.BasicNodeType == ProjectTreeNode.BasicNodeTypes.File)
                            {
                                System.IO.File.Move(sourceFile, destinationFile);
                            }
                            else if (sourceNode.BasicNodeType == ProjectTreeNode.BasicNodeTypes.Folder)
                            {
                                Directory.Move(sourceFile, destinationFile);
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        sourceNode.Remove();
                        oldParentNode.Nodes.Add(sourceNode);
                        oldParentNode.Expand();
                        MessageBox.Show(ex.Message);
                    }

                    SaveProjectFile();
                }
            }
            else
            {
                try
                {
                    string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
                    if (files != null)
                    {
                        foreach (string fullFileAndPath in files)
                        {
                            if (Path.GetExtension(fullFileAndPath).ToLower() == global.ProjectFileExtension)
                            {
                                LoadProjectFile(fullFileAndPath);
                            }
                            else
                            {
                                AddNewTab(fullFileAndPath);
                            }
                        }
                    }
                }
                catch { }
            }
        }

        #endregion

        #region Save, Load and Close Project

        private bool CreateEmptyProject(string fileName)
        {
            _ProjectTree.Nodes.Clear();
            ProjectFileName = fileName;

            try
            {
                ProjectTreeNode node = new ProjectTreeNode();
                node.Text = Path.GetFileNameWithoutExtension(fileName);
                node.NodeType = ProjectTreeNode.NodeTypes.Project;
                _ProjectTree.Nodes.Add(node);
                if (SaveProjectFile())
                {
                    return LoadProjectFile(fileName);
                }
            }
            catch
            {
            }

            ProjectFileName = null;

            return false;
        }

        public void CloseProject()
        {
            if (IsProjectOpen())
            {
                if (CloseAllTabs())
                {
                    AddDefaultProjectNode();
                }
            }
        }

        void AddDefaultProjectNode()
        {
            _ProjectTree.Nodes.Clear();
            LoadProjectImageList();
            ProjectTreeNode node = new ProjectTreeNode();
            node.NodeType = ProjectTreeNode.NodeTypes.Invalid;
            node.SetImage(ProjectTreeNode.NodeTypes.Project);
            node.Text = "<no project loaded>";
            _ProjectTree.Nodes.Add(node);
        }

        public bool SaveProjectFile()
        {
            if (IsProjectOpen())
            {
                ProjectTreeNode projectNode = GetProjectNode();
                XmlTextWriter textWriter = new XmlTextWriter(ProjectFileName, System.Text.Encoding.ASCII);
                textWriter.WriteStartDocument();
                textWriter.WriteStartElement("Project");
                SaveNodes(_ProjectTree.Nodes, textWriter);
                textWriter.WriteEndElement();
                textWriter.Close();
            }
            return true;
        }

        private void SaveNodes(TreeNodeCollection nodesCollection, XmlTextWriter textWriter)
        {
            for (int i = 0; i < nodesCollection.Count; i++)
            {
                ProjectTreeNode node = (ProjectTreeNode)nodesCollection[i];
                textWriter.WriteStartElement("node");
                textWriter.WriteAttributeString("Name", node.Text);
                textWriter.WriteAttributeString("Type", node.NodeType.ToString());
                if (node.Nodes.Count > 0)
                {
                    SaveNodes(node.Nodes, textWriter);
                }
                textWriter.WriteEndElement();
            }
        }

        public bool LoadProjectFile(string fileName)
        {
            if (!CloseAllTabs())
            {
                return false;
            }

            _ProjectTree.Nodes.Clear();
            ProjectFileName = fileName;

            LoadProjectImageList();

            XmlTextReader reader = null;
            try
            {
                // disabling re-drawing of treeview till all nodes are added
                _ProjectTree.BeginUpdate();				
                reader = new XmlTextReader(fileName);

                ProjectTreeNode parentNode = null;
				
                while (reader.Read())
                {
                    if (reader.NodeType == XmlNodeType.Element)
                    {
                        if (reader.Name == "node")
                        {
                            ProjectTreeNode newNode = new ProjectTreeNode();
                            bool isEmptyElement = reader.IsEmptyElement;

                            // loading node attributes.
                            int attributeCount = reader.AttributeCount;
                            if (attributeCount > 0)
                            {
                                for (int i = 0; i < attributeCount; i++)
                                {
                                    reader.MoveToAttribute(i);
                                    SetAttributeValue(newNode, reader.Name, reader.Value);
                                }								
                            }

                            if (newNode.NodeType == ProjectTreeNode.NodeTypes.Invalid)
                            {
                                newNode.NodeType = ProjectTreeNode.DetermineNodeType(Path.GetExtension(newNode.Text));
                            }

                            // add new node to Parent Node or TreeView.
                            if (parentNode != null)
                            {
                                parentNode.Nodes.Add(newNode);
                            }
                            else
                            {
                                newNode.Text = Path.GetFileNameWithoutExtension(newNode.Text);
                                _ProjectTree.Nodes.Add(newNode);
                            }

                            // making current node 'ParentNode' if its not empty.
                            if (!isEmptyElement)
                            {
                                parentNode = newNode;
                            }
                        }						                    
                    }
                    // moving up to in TreeView if end tag is encountered.
                    else if (reader.NodeType == XmlNodeType.EndElement)
                    {
                        if (reader.Name == "node")
                        {
                            parentNode = (ProjectTreeNode)parentNode.Parent;
                        }
                    }
                    else if (reader.NodeType == XmlNodeType.XmlDeclaration)
                    { //Ignore Xml Declaration.
                    }
                    else if (reader.NodeType == XmlNodeType.None)
                    {
                        return false;
                    }
                    else if (reader.NodeType == XmlNodeType.Text)
                    {
                        parentNode.Nodes.Add(reader.Value);
                    }

                }
            }
            finally
            {
                // enabling redrawing of treeview after all nodes are added.
                _ProjectTree.EndUpdate();      
                reader.Close();	
            }

            if (_ProjectTree.Nodes.Count > 0)
            {
                _ProjectTree.Nodes[0].Expand();
            }

            return true;
        }

        private void SetAttributeValue(ProjectTreeNode node, string propertyName, string value)
        {
            if (propertyName == "Name")
            {                
                node.Text = value;
            }
            else if (propertyName == "Type")
            {
                node.NodeType = (ProjectTreeNode.NodeTypes)Enum.Parse(typeof(ProjectTreeNode.NodeTypes), value);
                node.ImageKey = ProjectTreeNode.GetNodeTypeImageKey(node.NodeType);
                node.SelectedImageKey = node.ImageKey;
            }	
        }

        #endregion

        private ProjectTreeNode FindDefaultRunProjectFile()
        {
            foreach (TreeNode node in _ProjectTree.Nodes)
            {
                ProjectTreeNode foundNode = FindDefaultRunProjectFile(node);
                if (foundNode != null)
                {
                    return foundNode;
                }
            }
            return null;
        }

        private ProjectTreeNode FindDefaultRunProjectFile(TreeNode node)
        {
            if (((ProjectTreeNode)node).IsStartupObject)
            {
                return ((ProjectTreeNode)node);
            }

            foreach (ProjectTreeNode n in node.Nodes)
            {
                ProjectTreeNode foundNode = FindDefaultRunProjectFile(n);
                if (foundNode != null)
                {
                    return foundNode;
                }
            }
            return null;
        }

        bool CloseAffectedProjectTabs(ProjectTreeNode node)
        {
            string nodePath = node.LogicalPath();

            if (node.BasicNodeType == ProjectTreeNode.BasicNodeTypes.File)
            {
                DocTabPage docTabPag = GetTabByFilename(nodePath);
                if (docTabPag != null)
                {
                    return CloseTab(docTabPag);
                }
            }
            else if (node.BasicNodeType == ProjectTreeNode.BasicNodeTypes.Folder)
            {
                for (int tabIndex = 0; tabIndex < _TabDocs.TabPages.Count; tabIndex++)
                {
                    DocTabPage tabPage = (DocTabPage)_TabDocs.TabPages[tabIndex];

                    if (tabPage.CodeEditor.FileName != null && tabPage.CodeEditor.FileName.ToLower().StartsWith(nodePath.ToLower()))
                    {
                        if (!CloseTab(tabPage))
                        {
                            return false;
                        }
                    }
                    else if (tabPage.TempFileName != null && tabPage.TempFileName.ToLower().StartsWith(nodePath.ToLower()))
                    {
                        if (!CloseTab(tabPage))
                        {
                            return false;
                        }
                    }
                }
            }
            
            return true;
        }

        public ProjectTreeNode GetProjectNode()
        {
            if (_ProjectTree.Nodes.Count > 0)
            {
                if (((ProjectTreeNode)_ProjectTree.Nodes[0]).BasicNodeType == ProjectTreeNode.BasicNodeTypes.Project)
                {
                    return (ProjectTreeNode)_ProjectTree.Nodes[0];
                }
            }
            return null;
        }

        public bool IsProjectOpen()
        {
            return GetProjectNode() != null;
        }

        public string ProjectFileName
        {
            get { return _ProjectTree.Tag.ToString(); }
            set { _ProjectTree.Tag = value; }
        }

        void LoadProjectImageList()
        {
            if (_ProjectTree.ImageList == null)
            {
                _ProjectTree.ImageList = new ImageList();
                _ProjectTree.ImageList.ColorDepth = ColorDepth.Depth32Bit;
                _ProjectTree.ImageList.Images.Add("Project", global::SSIDE.Properties.Resources.TreeProjectFile);
                _ProjectTree.ImageList.Images.Add("Folder", global::SSIDE.Properties.Resources.TreeFolderFile);
                _ProjectTree.ImageList.Images.Add("CodeFile", global::SSIDE.Properties.Resources.TreeCodeFile);
                _ProjectTree.ImageList.Images.Add("CodeFileDefault", global::SSIDE.Properties.Resources.TreeCodeFileDefault);
                _ProjectTree.ImageList.Images.Add("ImageFile", global::SSIDE.Properties.Resources.TreeImageFile);
                _ProjectTree.ImageList.Images.Add("HTMLFile", global::SSIDE.Properties.Resources.TreeHTMLFile);
                _ProjectTree.ImageList.Images.Add("ScriptFile", global::SSIDE.Properties.Resources.TreeScriptFile);
                _ProjectTree.ImageList.Images.Add("StyleFile", global::SSIDE.Properties.Resources.TreeStyleFile);
                _ProjectTree.ImageList.Images.Add("JavaScript", global::SSIDE.Properties.Resources.TreeJavaScript);
                _ProjectTree.ImageList.Images.Add("XMLFile", global::SSIDE.Properties.Resources.TreeXMLFile);
                _ProjectTree.ImageList.Images.Add("BatchFile", global::SSIDE.Properties.Resources.TreeBatchFile);
                _ProjectTree.ImageList.Images.Add("TextFile", global::SSIDE.Properties.Resources.TreeTextFile);
                _ProjectTree.ImageList.Images.Add("UnknownFile", global::SSIDE.Properties.Resources.TreeUnknownFile);
            }
        }

        ProjectTreeNode AddExstingDirectory(DirectoryInfo source, DirectoryInfo target, ProjectTreeNode parentNode, bool recursive)
        {
            ProjectTreeNode node = null;

            string targetName = target.Name;

            foreach (ProjectTreeNode child in parentNode.Nodes)
            {
                if (child.Text.ToLower() == targetName.ToLower())
                {
                    node = child;
                    break;
                }
            }

            if (node == null)
            {
                node = parentNode.AddFolder(targetName);
            }

            // Check if the target directory exists, if not, create it.
            if (Directory.Exists(target.FullName) == false)
            {
                Directory.CreateDirectory(target.FullName);
            }

            // Copy each file into it’s new directory.
            foreach (FileInfo fi in source.GetFiles())
            {
                string targetFile = Path.Combine(target.ToString(), fi.Name);

                node.AddFile(fi.Name);

                if (!System.IO.File.Exists(targetFile))
                {
                    fi.CopyTo(targetFile, false);
                }
            }

            if (recursive)
            {
                // Copy each subdirectory using recursion.
                foreach (DirectoryInfo diSourceSubDir in source.GetDirectories())
                {
                    DirectoryInfo nextTargetSubDir = target.CreateSubdirectory(diSourceSubDir.Name);
                    AddExstingDirectory(diSourceSubDir, nextTargetSubDir, node, recursive);
                }
            }

            return node;
        }
    }
}
