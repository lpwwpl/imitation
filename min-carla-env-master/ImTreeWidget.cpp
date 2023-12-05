#include "ImTreeWidget.h"
#include <QStandardItem>
#include <QDir>
#include <QApplication>
#include <QDomDocument>
#include <QMouseEvent>
#include <QPainter>
#include <QVariant>
#include <QPixmap>
#include <QMenu>
#include <QDrag>
#include <QTreeWidgetItem>
#include <Gui/BitmapFactory.h>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "Mod/Robot/App/JointV.h"
#include "Mod/Robot/App/Targetpoint.h"

using namespace RobotGui;
using namespace Gui;

ImTreeNode::ImTreeNode(QString id, ImTreeNode* parent) :m_pParentNode(parent)
{
	if (m_pParentNode)
	{
		QAbstractItemModel* model = m_pParentNode->model();
		setModel(model);
	}
	m_index = QModelIndex();
	this->m_strName = "";
	this->m_strID = id;
}

ImTreeNode::ImTreeNode(const ImTreeNode &other)
{
	foreach(ImTreeNode *node, other.m_listChildren)
	{
		addChild(node->clone());
	}
	m_pParentNode = NULL;
	m_strName = other.m_strName;
	m_strID = other.m_strID;
}

ImTreeNode::~ImTreeNode()
{
	removeAllChild();
}

void ImTreeNode::setName(const QString &newName)
{
	m_strName = newName;
}

QString ImTreeNode::getName()
{
	return m_strName;
}

void ImTreeNode::setParent(ImTreeNode *parent)
{
	this->m_pParentNode = parent;
}

ImTreeNode* ImTreeNode::getParent()
{
	return m_pParentNode;
}
int ImTreeNode::row()
{
	if (m_pParentNode)
	{
		return m_pParentNode->indexOf(const_cast<ImTreeNode*>(this));
	}
	return 0;
}
void ImTreeNode::addChild(ImTreeNode *child)
{
	child->setParent(this);
	m_listChildren.append(child);
}

void ImTreeNode::insertChildAt(ImTreeNode *child, int index)
{
	child->setParent(this);
	m_listChildren.insert(index, child);
}

bool ImTreeNode::removeChild(ImTreeNode *child)
{
	if (child == NULL)
		return false;
	QList<ImTreeNode*>::iterator iter = m_listChildren.begin();
	while (iter != m_listChildren.end())
	{
		if (*iter == child)
		{
			m_listChildren.erase(iter);		
			delete child;
			child = NULL;
			return true;
		}
		else
		{
			++iter;
		}
	}
	return false;
}
bool ImTreeNode::detachChild(ImTreeNode* child)
{
	if (child == NULL)
		return false;

	QList<ImTreeNode*>::iterator iter = m_listChildren.begin();
	while (iter != m_listChildren.end())
	{
		if (*iter == child)
		{
			iter = m_listChildren.erase(iter);
			child->setParent(NULL);
			return true;
		}
		else
		{
			++iter;
		}
	}
	return false;
}

int ImTreeNode::getChildCount()
{
	return m_listChildren.count();
}

bool ImTreeNode::moveChildToFront(ImTreeNode *child)
{
	if (detachChild(child))
	{
		insertChildAt(child, 0);
		return true;
	}
	return false;
}

ImTreeNode* ImTreeNode::getChild(int index)
{
	return m_listChildren[index];
}

ImTreeNode* ImTreeNode::findChildByID(QString id)
{
	foreach(ImTreeNode *node, m_listChildren)
	{
		if (node && node->getID() == id)
		{
			return node;
		}
	}
	return NULL;
}

ImTreeNode* ImTreeNode::findChildByName(QString name)
{
	foreach(ImTreeNode *node, m_listChildren)
	{
		if (node && node->getName() == name)
		{
			return node;
		}
	}
	return NULL;
}

int ImTreeNode::indexOf(ImTreeNode *child)
{
	return m_listChildren.indexOf(child);
}

int ImTreeNode::getChildNodeCount()
{
	int count = 0;
	foreach(ImTreeNode *childNode, m_listChildren)
	{
		count += childNode->getChildNodeCount();
		count += 1;
	}
	return count;
}


void ImTreeNode::removeAllChild()
{
	foreach(ImTreeNode *node, m_listChildren)
	{
		delete node;
	}
	m_listChildren.clear();
}


int ImTreeNode::getFieldCount()
{
	return m_fields.size();
}

/////////////////////////////////
ImTreeRootNode::ImTreeRootNode(QString name, ImTreeNode* parent) : ImTreeNode(name,parent)
{

}

ImTargetNode::ImTargetNode(QString name, ImTreeNode* parent) : ImTreeNode(name, parent)
{

}
ImTargetRootNode::ImTargetRootNode(QString name, ImTreeNode* parent) : ImTreeNode(name, parent)
{

}
ImJointRootNode::ImJointRootNode(QString name, ImTreeNode* parent) : ImTreeNode(name, parent)
{

}
ImJointNode::ImJointNode(QString name, ImTreeNode* parent) : ImTreeNode(name, parent)
{

}
ImFrameOfNode::ImFrameOfNode(QString name, ImTreeNode* parent) : ImTreeNode(name, parent)
{

}
///////////////////////////////////////////////////////
ImTreeNode::ENodeType ImTreeRootNode::getNodeType()
{
	return ImTreeNode::eRootNode;
}
ImTreeNode* ImTreeRootNode::clone()
{
	return new ImTreeRootNode(*this);
}
///////////////////////////////////////////////////////
ImTreeNode::ENodeType ImTargetNode::getNodeType()
{
	return ImTreeNode::eTargetNode;
}
ImTreeNode* ImTargetNode::clone()
{
	return new ImTargetNode(*this);
}
///////////////////////////////////////////////////////

ImTreeNode::ENodeType ImFrameOfNode::getNodeType()
{
	return ImTreeNode::eTargetNode;
}
ImTreeNode* ImFrameOfNode::clone()
{
	return new ImFrameOfNode(*this);
}

///////////////////////////////////////////////////////
ImTreeNode::ENodeType ImTargetRootNode::getNodeType()
{
	return ImTreeNode::eTargetRootNode;
}
ImTreeNode* ImTargetRootNode::clone()
{
	return new ImTargetRootNode(*this);
}

///////////////////////////////////////////////////////
ImTreeNode::ENodeType ImJointRootNode::getNodeType()
{
	return ImTreeNode::eJointRootNode;
}
ImTreeNode* ImJointRootNode::clone()
{
	return new ImJointRootNode(*this);
}

///////////////////////////////////////////////////////
ImTreeNode* ImJointNode::clone()
{
	return new ImJointNode(*this);
}
ImTreeNode::ENodeType ImJointNode::getNodeType()
{
	return ImTreeNode::eJointNode;
}
///////////////////////////////////////////////

QVariant ImTreeNode::getField(int index)
{
	return m_fields[index];
}
void ImTreeNode::setField(int index, QVariant value)
{
	if (index < 0) return;
	m_fields[index] = value;
}

//////////////////////////////////////////////

ImTreeModel::ImTreeModel(QObject *parent)
	: QAbstractItemModel(parent)
	, m_bNeedUpdate(true)
{
	m_pTreeRoot = NULL;
}

ImTreeRootNode* ImTreeModel::setTreeRoot(ImTreeRootNode *newTreeRoot)
{
	beginResetModel();
	ImTreeRootNode *oldTreeRoot = m_pTreeRoot;
	m_pTreeRoot = newTreeRoot;
	m_pTreeRoot->setModel(this);
	endResetModel();
	return oldTreeRoot;
}

ImTreeRootNode *ImTreeModel::getTreeRoot()
{
	return m_pTreeRoot;
}


void ImTreeModel::needUpdate()
{
	m_bNeedUpdate = true;
}

void ImTreeModel::update()
{
	if (m_bNeedUpdate)
	{
		//reset();
		beginResetModel();
		endResetModel();
		m_bNeedUpdate = false;
	}
}

void ImTreeModel::clear()
{
	m_pTreeRoot->removeAllChild();
}

QModelIndex ImTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!m_pTreeRoot)
	{
		return QModelIndex();
	}

	if (!hasIndex(row, column, parent)) { return QModelIndex(); }
	ImTreeNode* parentNode = NULL;

	if (parent.isValid()){
		parentNode = nodeFromIndex(parent);
	}
	else {
		parentNode = m_pTreeRoot;
	}
	ImTreeNode* childNode = parentNode->getChild(row);
	if (childNode)
	{
		QModelIndex modelIndex = createIndex(row, column, childNode);
		//只有一列，
		if (column == 0){
			if (childNode->modelIndex() != modelIndex){
				childNode->setModelIndex(modelIndex);
			}
		}
		return modelIndex;
	}
	
	return QModelIndex();
}

QModelIndex ImTreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid()) { return QModelIndex(); }
	ImTreeNode *c = static_cast<ImTreeNode *>(index.internalPointer());
	ImTreeNode *p = c->getParent();
	if (p == m_pTreeRoot) { return QModelIndex(); }
	return createIndex(p->row(), 0, p);

/*	if (!index.isValid()) return QModelIndex();

	ImTreeNode* childNode = nodeFromIndex(index);
	if (!childNode) return QModelIndex();

	ImTreeNode* parentNode = childNode->getParent();
	if ((parentNode == m_pTreeRoot) || (!parentNode)) return QModelIndex();
	return createIndex(parentNode->row(), 0, parentNode);*/
}

int ImTreeModel::rowCount(const QModelIndex &parent) const
{
	if (!m_pTreeRoot) return 0;
	ImTreeNode *parentNode = NULL;
	if (parent.isValid())
		parentNode = nodeFromIndex(parent);
	else
		parentNode = m_pTreeRoot;
	return parentNode->getChildCount();
}

int ImTreeModel::columnCount(const QModelIndex &parent) const
{
	if (m_pTreeRoot == NULL)
	{
		return 0;
	}

	ImTreeNode *node = nodeFromIndex(parent);
	if (NULL == node)
	{
		node = m_pTreeRoot;
	}
	return 1;// node->getFieldCount();
}

QVariant ImTreeModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	int col = index.column();
	if (m_pTreeRoot == NULL)
	{
		return QVariant();
	}

	ImTreeNode* node = nodeFromIndex(index);
	if (node == NULL)
	{
		return QVariant();
	}
	switch (role)
	{
	case Qt::DisplayRole:
		if (index.column() == 0){
			return node->getField(0);
		}
		else return QVariant();		
	case Qt::DecorationRole:
	{
		return QVariant();
		//QString type = node->getField(1).toString();
		//if (type == "package")
		//	return QIcon(QStringLiteral(":/report/images/package.png"));
		//else if (type == "finding")
		//	return QIcon(QStringLiteral(":/report/images/findings.png"));
		//else
		//	return QVariant();
	}
	case Qt::TextAlignmentRole:
	{
		if (index.column() == 0)
		{
			return Qt::AlignLeft;
		}
	}
	case Qt::TextColorRole:
		return QColor(Qt::black);
	case Qt::UserRole:
		return node->getField(1);
		//break;
	case Qt::UserRole + 1:
		return node->getField(2);
		//break;
	default:
		break;
	}

	return QVariant();
}

Qt::ItemFlags ImTreeModel::flags(const QModelIndex &index) const
{
	if (NULL == m_pTreeRoot)
	{
		return 0;
	}

	if (!index.isValid())
	{
		return 0;
	}

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}


ImTreeNode* ImTreeModel::nodeFromIndex(const QModelIndex& index) const
{
	if (index.isValid()){
		return static_cast<ImTreeNode*>(index.internalPointer());
	}
	else return m_pTreeRoot;
}



void ImTreeProxyWidget::SlotTreeViewDoubleClick(const QModelIndex & index)
{
	QAbstractItemModel* m = (QAbstractItemModel*)index.model();

	ImTreeModel* model = dynamic_cast<ImTreeModel*>(m);

	
	QVariant infoVar = model->data(index, Qt::UserRole+1);
	QVariant varType = model->data(index, Qt::UserRole);

	//SFindingInfo info = infoVar.value<SFindingInfo>();

	//if (varType == "finding")
	//{
	//	emit signalReportFindingsString(info.m_desc, info.m_result);
	//}

	//if package ,don't emit
}
//TaskBox(Gui::BitmapFactory().pixmap("document-new"), tr("ImTree"), true, parent)
ImTreeProxyWidget::ImTreeProxyWidget(Robot::RobotObject* pcRobotObject,Robot::TargetObject* pcTargetObject, QWidget *parent )
	:QDialog(parent),pcRobot(pcRobotObject),pcTarget(pcTargetObject)
{
	m_rootNode = new ImTreeRootNode("Target",0);
	m_rootJointVNode = new ImJointRootNode("Joints",m_rootNode);
	m_rootJointVNode->setField(0,"Joints");
	m_rootTargetpointNode = new ImTargetRootNode("TargetPoints", m_rootNode);
	m_rootTargetpointNode->setField(0, "TargetPoints");
	m_treeView = new ImTreeView();	
	//m_treeView->setParent(this);
	m_treeView->setRobot(pcRobotObject);
	m_treeView->setTarget(pcTargetObject);

	m_model = new ImTreeModel(m_treeView);
	m_model->setTreeRoot(m_rootNode);
	m_rootNode->setModel(m_model);
	m_rootNode->addChild(m_rootJointVNode);
	m_rootNode->addChild(m_rootTargetpointNode);
	//Init();


	m_treeView->setModel(m_model);
	m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);///////////////////
	m_treeView->setAlternatingRowColors(true);
	m_treeView->setSortingEnabled(true);
	m_treeView->setRootIsDecorated(true);
	m_treeView->setHeaderHidden(true);
	m_treeView->setSelectionBehavior(QTreeView::SelectRows);			
	m_treeView->setSelectionMode(QTreeView::SingleSelection);
	m_treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	connect(m_treeView, SIGNAL(doubleClicked(const QModelIndex)), this, SLOT(SlotTreeViewDoubleClick(const QModelIndex)));

	m_hLayout = new QHBoxLayout(this);
	m_hLayout->addWidget(m_treeView);
	m_hLayout->setMargin(0);
	setLayout(m_hLayout);

	//启用拖放
	m_treeView->setDragEnabled(true);
	//设置拖放
	m_treeView->setAcceptDrops(true);
	//设置显示将要被放置的位置
	m_treeView->setDropIndicatorShown(true);
	//设置拖放模式为移动项目，否则为复制项目
	m_treeView->setDragDropMode(QAbstractItemView::DragDrop);

	//buildtree(); //m_rootNode
	m_treeView->expandAll();
	update();
}
void ImTreeProxyWidget::buildtree()
{

	if (!pcTarget)return;
	//rootNode->removeAllChild();

	const std::vector<Robot::JointV*>& jointVs = pcTarget->JointVs.getValue();
	const std::vector<Robot::Targetpoint*>& targetpoints = pcTarget->Targetpoints.getValue();

	for (Robot::JointV* elem : jointVs)
	{
		ImJointNode* jNode = new ImJointNode(QString::fromStdString(elem->Name),m_rootJointVNode);
	}

	for (Robot::Targetpoint* elem : targetpoints)
	{
		ImTargetNode* tNode = new ImTargetNode(QString::fromStdString(elem->Name), m_rootTargetpointNode);
		m_rootTargetpointNode->addChild(tNode);
	}
}
void ImTreeProxyWidget::refresh()
{
	m_treeView->updateModel();
}

ImTreeProxyWidget::~ImTreeProxyWidget()
{
	//SAFE_RELEASE(m_model);
	//SAFE_RELEASE(m_treeView);
	//SAFE_RELEASE(m_rootNode);
	//SAFE_RELEASE(m_hLayout);
}



/// <summary>
/// //////////////////////////////////////////
/// </summary>
/// <param name="parent"></param>
ImTreeView::ImTreeView(QWidget* parent) : _Base(parent)
{	
	setStyleSheet(""
		"QTreeView {outline:none;show-decoration-selected: 1;}"
		"QTreeView {outline:none;border:0px;}"
		"QTreeView::branch{		background-color: transparent;	}"
		"QTreeView::item:hover, QTreeView::branch:hover { background-color: transparent;border-color: rgb(255, 0, 0);}"
		"QTreeView::item:selected, QTreeView::branch:selected { background-color: #C5E0F7;}"
		"QTreeView::branch:open:has-children{image: url(:/icons/branch-open.png);}"
		"QTreeView::branch:closed:has-children{image: url(:/icons/branch-closed.png);}"
		"QTreeView::branch:has-siblings:!adjoins-item{border-image:url(:/icons/vline.png) 0;}"
		"QTreeView::branch:has-siblings:adjoins-item{border-image:url(:/icons/branch-more.png) 0;}"
		"QTreeView::branch:!has-children:!has-siblings:adjoins-item{border-image:url(:/icons/branch-end.png) 0;	}"
		"QTreeView::branch:has-children:!has-siblings:closed,QTreeView::branch:closed:has-children:has-siblings{border-image:none;image: url(:/icons/branch-closed.png);	}"
		"QTreeView::branch:open:has-children:!has-siblings,QTreeView::branch:open:has-children:has-siblings{border-image:none;image: url(:/icons/branch-open.png);	}"
	);

	m_node = NULL;
	m_pAdd = NULL;
	m_pDel = NULL;
	m_pEdit = NULL;
	m_pContextMenu = NULL;
	//, delegate_(new ImReportFindingsItemDelegate(this))
	//setItemDelegate(delegate_);
	//connect(delegate_, SIGNAL(expand(const QModelIndex&)), this, SLOT(onDeledateExpand(const QModelIndex&)));
	createMenu();
}
//void ImTreeView::setModel(ImTreeModel* model)
//{
//	m_model = model;
//}

//----------------------------------------------------------------------------------------------------------------------
void ImTreeView::onDeledateExpand(const QModelIndex& index)
{
	if (isExpanded(index))
		collapse(index);
	else
		expand(index);
}
void ImTreeView::updateModel()
{
	QAbstractItemModel* model = this->model();
	ImTreeModel* treeModel = dynamic_cast<ImTreeModel*>(model);
	if (treeModel)
	{
		treeModel->update();
		expandAll();
	}
	
}
void ImTreeView::createMenu()
{
	m_pContextMenu = new QMenu(this);

	m_pAdd = new QAction(tr("Add"), this);
	if (m_pAdd)
	{
		connect(m_pAdd, SIGNAL(triggered(bool)), this, SLOT(slotAddAction()));
	}
	m_pDel = new QAction(tr("Delete"), this);
	if (m_pDel)
	{
		connect(m_pDel, SIGNAL(triggered(bool)), this, SLOT(slotDeleteAction()));
	}
	m_pEdit = new QAction(tr("Edit"), this);
	if (m_pEdit)
	{
		connect(m_pEdit, SIGNAL(triggered(bool)), this, SLOT(slotEditAction()));
	}
}
void ImTreeView::slotAddAction()
{
	if (m_node)
	{
		//m_editor->setNode(m_node);
		//m_editor->setOperateType(eAdd);
		ImTreeNode::ENodeType nodeType = m_node->getNodeType();
		if (nodeType == ImTreeNode::eTargetRootNode)
		{
			const Base::Placement& tcp = pcRobot->Tcp.getValue();

		}
		else if (nodeType == ImTreeNode::eJointRootNode)
		{

		}
	}
}
void ImTreeView::slotDeleteAction()
{
	if (m_node)
	{
		ImTreeNode::ENodeType nodeType = m_node->getNodeType();
		QVariant var = m_node->getField(2);
		//SFindingInfo info = var.value<SFindingInfo>();
		QAbstractItemModel* model = m_node->model();
		ImTreeModel* treeModel = dynamic_cast<ImTreeModel*>(model);
		if (!treeModel)return;

	
		if (m_node->getNodeType() == ImTreeNode::eTargetNode)
		{

			//if (1)
			//{
			//	treeModel->needUpdate();
			//	treeModel->m_pTreeRoot->removeChild(m_node);
			//	updateModel();
			//}
		}
		else if (m_node->getNodeType() == ImTreeNode::eTargetNode)
		{
		
		}
	}
}

void ImTreeView::slotEditAction()
{
	
}

void ImTreeView::startDrag(Qt::DropActions supportedActions)
{
	QModelIndex currentIndex = this->currentIndex();
	QAbstractItemModel* model = this->model();
	ImTreeModel* treeModel = dynamic_cast<ImTreeModel*>(model);
	if (currentIndex.isValid())
	{
		ImTreeNode* node = treeModel->nodeFromIndex(currentIndex);
	/*	if (node->getNodeType() != ImTreeNode::eSubDetailNode) return;*/
		TreeItemMimeData *mimeData = new TreeItemMimeData;
		mimeData->SetDragData("ItemMimeData", currentIndex);

		QDrag *drag = new QDrag(this);
		QPixmap pixmap(":/report/images/findings.png");
		drag->setMimeData(mimeData);
		drag->setPixmap(pixmap);
		drag->setHotSpot(QPoint(pixmap.width() / 2, pixmap.height() / 2));
		drag->exec(Qt::CopyAction);
	}
	//QMimeData * mime = new QMimeData;
	//QByteArray byteArray;
	//QDataStream dataStream(&byteArray, QIODevice::WriteOnly);

	//QListWidgetItem * item = currentItem();

	//QString string = item->data(Qt::UserRole).value<QString>();
	//QPixmap pixmap = item->data(Qt::UserRole + 1).value<QPixmap>();

	//dataStream << string << pixmap;
	//mime->setData("application/x-draganddrop", byteArray);

	//QDrag * drag = new QDrag(this);
	//drag->setPixmap(pixmap);
	//drag->setMimeData(mime);
	//drag->setHotSpot(QPoint(pixmap.width() / 2, pixmap.height() / 2));
}

void ImTreeView::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("ItemMimeData"))
	{
		event->setDropAction(Qt::CopyAction);
		event->accept();
	}		
	else
		event->ignore();
}
/*void ImTreeView::dragLeaveEvent(QDragLeaveEvent *event)
{
	QRect updateRect = highlightedRect;
	highlightedRect = QRect();
	update(updateRect);
	event->accept();
}*/
void ImTreeView::dragMoveEvent(QDragMoveEvent  *event)
{
	if (event->mimeData()->hasFormat("ItemMimeData")) 
	{
		const TreeItemMimeData *pMimeData = (const TreeItemMimeData *)(event->mimeData());
		QModelIndex item = pMimeData->DragItemData();
		QModelIndex currentIndex = this->indexAt(event->pos());
		if (currentIndex == item)           //不允许拖回到原来的item  
		{
			event->ignore();
		}
		else
		{
			event->setDropAction(Qt::CopyAction);
			event->accept();
		}
	}
	else
		event->ignore();
}
void ImTreeView::mouseMoveEvent(QMouseEvent *event)
{
	QTreeView::mouseMoveEvent(event);
}

void ImTreeView::mousePressEvent(QMouseEvent *event)
{
	QTreeView::mousePressEvent(event);
}
//https://www.cnblogs.com/findumars/p/5176057.html
//https://doc.qt.io/archives/4.6/itemviews-puzzle-piecesmodel-cpp.html

void ImTreeView::dropEvent(QDropEvent *event)
{
	QAbstractItemModel* model = this->model();
	ImTreeModel* treeModel = dynamic_cast<ImTreeModel*>(model);

	if (event->mimeData()->hasFormat("ItemMimeData"))
	{
		QByteArray itemData = event->mimeData()->data("ItemMimeData");
		const TreeItemMimeData *pMimeData = (const TreeItemMimeData *)(event->mimeData());
		QModelIndex dragIndex = pMimeData->DragItemData();
		
		QModelIndex currentIndex = this->indexAt(event->pos());

		if (currentIndex.isValid() && (currentIndex != dragIndex))
		{
			ImTreeNode *pNode = treeModel->nodeFromIndex(currentIndex);
			if (!pNode)return;
		/*	ImTreeNode::ENodeType nodeType = pNode->getNodeType();
			if (nodeType == ImTreeNode::eSubDetailNode)
			{
				pNode = pNode->getParent();
			}
			if (!pNode)return;
			ImTreeNode *pDragNode = treeModel->nodeFromIndex(dragIndex);
			if (!pDragNode)return;*/
			//if (pNode->getNodeType() == ImTreeNode::ePackageNode)
			{
				/*SFindingInfo info = pDragNode->getField(2).value<SFindingInfo>();
				SFindingInfo clone_info = info.clone();
				clone_info.m_category = pNode->getField(0).toString();
				bool ret = ImReportFindingXmlUtil::getInstance()->addInfo(clone_info);
				if (ret)
				{
					ImTreeNode* subNode = new ImSubDetailNode("finding", pNode);
					subNode->setField(0, info.m_name);
					subNode->setField(1, "finding");
					subNode->setField(2, QVariant().fromValue(clone_info));
					pNode->addChild(subNode);
					treeModel->needUpdate();
					updateModel();
				}
				else
				{
					ImQMessageBox::information(this, tr("Diag Node"), tr("Failed!Diag Node Template Exists!"), QMessageBox::Yes, ImLngLoader::LoadLng("00583"));				
				}*/
			}
		}
		else
		{
			
		}

		event->accept();
	}
	else
		event->ignore();
}

void ImTreeView::contextMenuEvent(QContextMenuEvent  *event)
{

	if (NULL == m_pContextMenu)
	{
		return;
	}
	m_pContextMenu->clear();
	QTreeWidgetItem *item = NULL;
	QPoint point = event->pos();

	QModelIndex index = indexAt(point);
	QAbstractItemModel* model = this->model();
	ImTreeModel* treeModel = dynamic_cast<ImTreeModel*>(model);
	ImTreeNode* node = NULL;
	if (treeModel)
	{
		node = treeModel->nodeFromIndex(index);
	}

	if (!node)
	{
		return;
	}
	m_node = node;
	ImTreeNode::ENodeType type = node->getNodeType();
	if (type == ImTreeNode::eTargetRootNode || type == ImTreeNode::eTargetRootNode)
	{
		m_pContextMenu->addAction(m_pAdd);
	}
	m_pContextMenu->addAction(m_pDel);
	m_pContextMenu->addAction(m_pEdit);

	m_pContextMenu->exec(QCursor::pos());
	event->accept();
}

QMimeData *ImTreeModel::mimeData(const QModelIndexList &indexes) const
{
	if (indexes.count() <= 0)
		return 0;

	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;

	QDataStream stream(&encodedData, QIODevice::WriteOnly);
	QModelIndex index = indexes.at(0);
	ImTreeNode* node = nodeFromIndex(index);
	if (!node)
	{
		return 0;
	}
	//QString name = node->getField(0).toString();
	//QString type = node->getField(1).toString();
	//SFindingInfo info = node->getField(2).value<SFindingInfo>();
	//
	////QPixmap pixmap = qVariantValue<QPixmap>(data(index, Qt::UserRole));
	//stream << name  << type << info;
	//mimeData->setData("application/ImReport", encodedData);

	return mimeData;
}

bool ImTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (action == Qt::IgnoreAction)
		return true;
	if (!data->hasFormat("application/ImReport"))
		return false;
	
	QByteArray encodedData = data->data("image/x-puzzle-piece");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);

	while (!stream.atEnd()) 
	{
		QString name;
		QString type; 
		//SFindingInfo info;
		stream >> name >> type;// >> info;
		ImTreeNode* pNode = nodeFromIndex(parent);

		if (!pNode)
		{
			return false;
		}
		if (pNode->getNodeType() == ImTreeNode::eTargetNode)
		{
			//bool ret = ImReportFindingXmlUtil::getInstance()->addInfo(info);
			//if (ret)
			//{
			//	ImTreeNode* subNode = new ImSubDetailNode("finding", pNode);
			//	subNode->setField(0, info.m_name);
			//	subNode->setField(1, "finding");
			//	subNode->setField(2, QVariant().fromValue(info));
			//	pNode->addChild(subNode);
			//}
		}
	}
	return true;
}

QStringList ImTreeModel::mimeTypes() const
{
	QStringList types;
	types << "application/ImReport";
	return types;
}




#include "moc_ImTreeWidget.cpp"