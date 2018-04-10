using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace SSIDE.Classes
{
    public class ProjectTreeNode : TreeNode
    {
        public bool IsStartupObject
        {
            get
            {
                return this.NodeType == NodeTypes.CodeFileDefault;
            }
            set
            {
                if (this.NodeType == NodeTypes.CodeFile || this.NodeType == NodeTypes.CodeFileDefault)
                {
                    if (value == true)
                    {
                        UnsetAllStartupObjects();
                    }

                    NodeTypes nodeType = value ? NodeTypes.CodeFileDefault : NodeTypes.CodeFile;
                    this.NodeType = nodeType;
                    this.SetImage(nodeType);
                }
            }
        }

        private void UnsetAllStartupObjects()
        {
            foreach (TreeNode node in this.TreeView.Nodes)
            {
                UnsetAllStartupObjects(node);
            }
        }

        private void UnsetAllStartupObjects(TreeNode node)
        {
            if (((ProjectTreeNode)node).IsStartupObject)
            {
                ((ProjectTreeNode)node).IsStartupObject = false;
            }

            foreach (ProjectTreeNode n in node.Nodes)
            {
                UnsetAllStartupObjects(n);
            }
        }

        private NodeTypes _NodeType = NodeTypes.Invalid;
        public NodeTypes NodeType
        {
            get { return _NodeType; }
            set
            {
                _NodeType = value;
                if (_NodeType == NodeTypes.Folder)
                {
                    _BasicNodeType = BasicNodeTypes.Folder;
                }
                else if (_NodeType == NodeTypes.Project)
                {
                    _BasicNodeType = BasicNodeTypes.Project;
                }
                else
                {
                    _BasicNodeType = BasicNodeTypes.File;
                }

                this.SetImage(NodeType);
            }
        }

        private BasicNodeTypes _BasicNodeType = BasicNodeTypes.Invalid;
        public BasicNodeTypes BasicNodeType
        {
            get { return _BasicNodeType; }
            set { BasicNodeType = value; }
        }

        public enum BasicNodeTypes
        {
            Invalid = 0,
            Project = 1,
            Folder = 2,
            File = 4
        }

        public enum NodeTypes
        {
            Invalid = 0,
            Project = 1,
            Folder = 2,
            CodeFile = 4,
            CodeFileDefault = 5,
            ImageFile = 6,
            HTMLFile = 7,
            StyleFile = 8,
            ScriptFile = 9,
            JavaScript = 10,
            XMLFile = 11, 
            BatchFile = 12,
            TextFile = 13,
            UnknownFile = 14
        }

        public ProjectTreeNode AddFile(string fileName)
        {
            ProjectTreeNode node = new ProjectTreeNode();
            node.NodeType = DetermineNodeType(fileName);
            node.ImageKey = GetNodeTypeImageKey(node.NodeType);
            node.SelectedImageKey = GetNodeTypeImageKey(node.NodeType);
            node.Text = Path.GetFileName(fileName);
            this.Nodes.Add(node);
            return node;
        }

        public ProjectTreeNode AddFolder(string nodeName)
        {
            string fileName = ((LogicalPath() + "\\" + nodeName).Replace("\\\\", "\\"));

            Directory.CreateDirectory(fileName);

            ProjectTreeNode node = new ProjectTreeNode();
            node.NodeType = NodeTypes.Folder;
            node.SetImage(node.NodeType);
            node.Text = nodeName;
            this.Nodes.Add(node);
            return node;
        }

        public ProjectTreeNode CreateNewFolder()
        {
            return AddFolder("New folder");
        }

        public ProjectTreeNode CreateNewFile(string extension)
        {
            string nodeName = "New file" + extension;
            string fileName = ((LogicalPath() + "\\" + nodeName).Replace("\\\\", "\\"));

            try
            {
                File.Create(fileName).Dispose();
            }
            catch (Exception ex)
            {

            }

            ProjectTreeNode node = new ProjectTreeNode();
            node.NodeType = DetermineNodeType(fileName);
            node.SetImage(node.NodeType);
            node.Text = nodeName;
            this.Nodes.Add(node);
            return node;
        }

        public ProjectTreeNode AddNode(string text, ProjectTreeNode.NodeTypes nodeType)
        {
            ProjectTreeNode node = new ProjectTreeNode();
            node.NodeType = nodeType;
            node.SetImage(nodeType);
            node.Text = text;
            this.Nodes.Add(node);
            return node;
        }

        public void SetImage(NodeTypes nodeType)
        {
            this.ImageKey = GetNodeTypeImageKey(nodeType);
            this.SelectedImageKey = GetNodeTypeImageKey(nodeType);
        }

        public string FileLineage(bool includeRoot)
        {
            string lineage = "";
            ProjectTreeNode parent = this;
            while(true)
            {
                if (includeRoot && parent == null)
                {
                    break;
                }
                else if(parent == parent.TreeView.Nodes[0])
                {
                    break;
                }

                lineage = "\\" + parent.Text + lineage;
                parent = (ProjectTreeNode)parent.Parent;
            }

            return lineage;
        }

        public string FileLineage()
        {
            return FileLineage(false);
        }

        public string LogicalPath()
        {
            string projectPath = Path.GetDirectoryName(this.TreeView.Tag.ToString());
            return (projectPath + "\\" + this.FileLineage()).Replace("\\\\", "\\");
        }

        static public ProjectTreeNode.NodeTypes DetermineNodeType(string fileName)
        {
            string fileExtension = Path.GetExtension(fileName).ToLower();

            if (fileExtension == global.CodeFileExtension)
            {
                return ProjectTreeNode.NodeTypes.CodeFile;
            }
            else if (fileExtension == ".jpg" || fileExtension == ".jpeg" || fileExtension == ".gif" || fileExtension == ".png"
                 || fileExtension == ".bmp" || fileExtension == ".tif" || fileExtension == ".tiff" || fileExtension == ".ico")
            {
                return ProjectTreeNode.NodeTypes.ImageFile;
            }
            else if (fileExtension == ".html" || fileExtension == ".htm" || fileExtension == ".ssi" || fileExtension == ".shtml")
            {
                return ProjectTreeNode.NodeTypes.HTMLFile;
            }
            else if (fileExtension == ".css")
            {
                return ProjectTreeNode.NodeTypes.StyleFile;
            }
            else if (fileExtension == ".xml" || fileExtension == ".xsl")
            {
                return ProjectTreeNode.NodeTypes.XMLFile;
            }
            else if (fileExtension == ".bat")
            {
                return ProjectTreeNode.NodeTypes.BatchFile;
            }
            else if (fileExtension == ".txt")
            {
                return ProjectTreeNode.NodeTypes.TextFile;
            }
            else if (fileExtension == ".js")
            {
                return ProjectTreeNode.NodeTypes.JavaScript;
            }
            else
            {
                return ProjectTreeNode.NodeTypes.UnknownFile;
            }
        }

        static public string GetNodeTypeImageKey(ProjectTreeNode.NodeTypes nodeType)
        {
            string typeString = nodeType.ToString();

            return typeString;
        }
    }
}
