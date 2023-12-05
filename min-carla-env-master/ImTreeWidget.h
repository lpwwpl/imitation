
#ifndef IMTREEWIDGET_H
#define IMTREEWIDGET_H

#include <QWidget>
#include <QStyledItemDelegate>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QDomDocument>
#include <QTextStream>
#include <QDebug>
#include <QTreeView>
#include <QDialog>
#include <Gui/TaskView/TaskView.h>
#include <Mod/Robot/App/RobotObject.h>
#include <Mod/Robot/App/TargetObject.h>
#include <QMimeData>

#define AS_NODE(CLASS,pNode) dynamic_cast<CLASS*>(pNode)
#define IS_NODE(CLASS,pNode) AS_NODE(CLASS,pNode) != NULL
#define SAFE_RELEASE(p) if(p) {delete p;p=NULL;}
#define SAFE_RELEASE_ARRAY(p) if(p) {delete []p; p = NULL;}

namespace RobotGui {

	class ImTreeRootNode;
	class ImTargetNode;
	class ImTargetRootNode;
	class ImJointNode;
	class ImJointRootNode;
	class ImTreeModel;
	class ImTreeItemDelegate;
	class ImTreeNode
	{
	public:
		ImTreeNode(QString m_strID, ImTreeNode* parent);
		ImTreeNode(const ImTreeNode& other);
		virtual ~ImTreeNode();

	public:
		virtual void setField(int index, QVariant value);
		virtual QVariant getField(int index);
		enum ENodeType
		{
			eNoneNode=0,
			eRootNode,
			eTargetNode,
			eTargetRootNode,
			eJointRootNode,
			eJointNode,
			eFrameOfRootNode,
			eFrameOfNode,
			eProgramNode,
			eProgramRootNode
		};

		virtual ImTreeNode::ENodeType getNodeType() = 0;

	public:
		virtual QString getID() { return m_strID; }

		void setName(const QString& m_strName);
		QString getName();

		void setParent(ImTreeNode* m_pParentNode);
		ImTreeNode* getParent();

		void addChild(ImTreeNode* child);
		void insertChildAt(ImTreeNode* child, int index);
		bool detachChild(ImTreeNode* child);
		int getChildCount();
		bool moveChildToFront(ImTreeNode* child);

		ImTreeNode* getChild(int index);

		ImTreeNode* findChildByID(QString m_strID);
		ImTreeNode* findChildByName(QString m_strName);

		bool removeChild(ImTreeNode* child);
		void removeAllChild();
		int indexOf(ImTreeNode* child);
		int getChildNodeCount();
		int row();
		void setModel(QAbstractItemModel* model) { m_model = model; }
		QAbstractItemModel* model() { return m_model; }
		void setModelIndex(const QModelIndex& index) { m_index = index; }
		QModelIndex modelIndex() { return m_index; }

	public:
		virtual ImTreeNode* clone() = 0; // Must implement it.
		virtual int getFieldCount();

	public:
		typedef QList<ImTreeNode*>  TreeNodeArray;
		TreeNodeArray m_listChildren;
		ImTreeNode* m_pParentNode;
	protected:
		QMap<int, QVariant> m_fields;
		QString m_strName;
		QString m_strID;

		QAbstractItemModel* m_model;
		QModelIndex m_index;
	};


	//class ImJointNode;

	class ImTreeRootNode : public ImTreeNode
	{
	public:
		ImTreeRootNode(QString, ImTreeNode*);

	public:
		virtual ImTreeNode::ENodeType getNodeType();

	protected:
		virtual ImTreeNode* clone();
	};

	class ImTargetNode : public ImTreeNode
	{
	public:
		ImTargetNode(QString, ImTreeNode*);

	public:
		virtual ImTreeNode::ENodeType getNodeType();

	protected:
		virtual ImTreeNode* clone();
	};

	class ImTargetRootNode : public ImTreeNode
	{
	public:
		ImTargetRootNode(QString, ImTreeNode*);

	public:
		virtual ImTreeNode::ENodeType getNodeType();

	protected:
		virtual ImTreeNode* clone();
	};

	class ImJointRootNode : public ImTreeNode
	{
	public:
		ImJointRootNode(QString, ImTreeNode*);

	public:
		virtual ImTreeNode::ENodeType getNodeType();

	protected:
		virtual ImTreeNode* clone();
	};
	class ImJointNode : public ImTreeNode
	{
	public:
		ImJointNode(QString, ImTreeNode*);

	public:
		virtual ImTreeNode::ENodeType getNodeType();

	protected:
		virtual ImTreeNode* clone();
	};

	class ImFrameOfNode : public ImTreeNode
	{
	public:
		ImFrameOfNode(QString, ImTreeNode*);

	public:
		virtual ImTreeNode::ENodeType getNodeType();

	protected:
		virtual ImTreeNode* clone();
	};

	class ImTreeModel : public QAbstractItemModel
	{
	public:
		ImTreeModel(QObject* parent = 0);

		ImTreeRootNode* setTreeRoot(ImTreeRootNode* m_pTreeRoot);
		ImTreeRootNode* getTreeRoot();

		void needUpdate();
		void update();
		void clear();

	public:
		QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex& child) const;
		int rowCount(const QModelIndex& parent = QModelIndex()) const;
		int columnCount(const QModelIndex& parent = QModelIndex()) const;
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		ImTreeNode* nodeFromIndex(const QModelIndex& index) const;
		Qt::ItemFlags flags(const QModelIndex& index) const;
		QMimeData *mimeData(const QModelIndexList &indexes) const;
		bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
	    QStringList mimeTypes() const;

	public:
		ImTreeRootNode* m_pTreeRoot;
	private:

		bool    m_bNeedUpdate;
	};


	class ImTreeView : public QTreeView
	{
		using _Base = QTreeView;
		Q_OBJECT
	public:
		ImTreeView(QWidget* parent = 0);
		virtual ~ImTreeView()
		{
			SAFE_RELEASE(m_node);
			SAFE_RELEASE(m_pAdd);
			SAFE_RELEASE(m_pDel);
			SAFE_RELEASE(m_pEdit);
			SAFE_RELEASE(m_pContextMenu);
		}
		//void setModel(ImTreeModel*);
		
		void updateModel();
		void dropEvent(QDropEvent* event);
		void startDrag(Qt::DropActions supportedActions);
		void dragMoveEvent(QDragMoveEvent* event);
		void dragEnterEvent(QDragEnterEvent* event);
		void mouseMoveEvent(QMouseEvent* event);
		void mousePressEvent(QMouseEvent* event);
		void setRobot(Robot::RobotObject* pcRobotObject)
		{
			pcRobot = pcRobotObject;
		}
		void setTarget(Robot::TargetObject* pcTargetObject)
		{
			pcTarget = pcTargetObject;
		}

	private Q_SLOTS:
		void onDeledateExpand(const QModelIndex&);

	private:
		// Disable copying
		ImTreeView(const ImTreeView&) = delete;
		ImTreeView& operator=(const ImTreeView&) = delete;
		void createMenu();
		void contextMenuEvent(QContextMenuEvent* event);

		//ImTreeModel* model_ = nullptr;
		//ImTreeItemDelegate* delegate_ = nullptr;
	public Q_SLOTS:
		void slotAddAction();
		void slotDeleteAction();
		void slotEditAction();
	Q_SIGNALS:
		void signalRefreshTree();
	protected:
		ImTreeNode* m_node;
		QAction* m_pAdd;
		QAction* m_pDel;
		QAction* m_pEdit;
		QMenu* m_pContextMenu;

	protected:
		Robot::RobotObject* pcRobot;
		Robot::TargetObject* pcTarget;
	};


	class Q_DECL_EXPORT ImTreeProxyWidget : public QDialog //public Gui::TaskView::TaskBox
	{
		Q_OBJECT

	public:
		explicit ImTreeProxyWidget(Robot::RobotObject* , Robot::TargetObject*,QWidget* parent = 0);
		~ImTreeProxyWidget();
		void setRobot(Robot::RobotObject* pcRobotObject)
		{
			pcRobot = pcRobotObject;
		}
		void setTarget(Robot::TargetObject* pcTargetObject)
		{
			pcTarget = pcTargetObject;
		}
		void buildtree(); // ImTreeNode * m_rootNode = nullptr
	public Q_SLOTS:
		void SlotTreeViewDoubleClick(const QModelIndex& index);
	private:
		ImTreeView* m_treeView;
		ImTreeModel* m_model;
		ImTreeRootNode* m_rootNode;

		ImTreeNode* m_rootJointVNode;
		ImTreeNode* m_rootTargetpointNode;
		QHBoxLayout* m_hLayout;

		//userRole
		//type
		//userRole+1
		//findings words
		void refresh();

	protected:
		Robot::RobotObject* pcRobot;
		Robot::TargetObject* pcTarget;
	};



	class ImTargetTreeWidget : public QObject
	{
		Q_OBJECT
	public:
		ImTargetTreeWidget();
		~virtual ImTargetTreeWidget();


	protected:
		QList<Q
	};
}


class TreeItemMimeData :public QMimeData
{
	Q_OBJECT
public:
	TreeItemMimeData() :QMimeData()
	{

	}

	~TreeItemMimeData()
	{

	}

	void SetDragData(QString mimeType, QModelIndex pItem)
	{

		m_format << mimeType;
		m_pDragItem = pItem;
	}



	QStringList formats() const
	{
		return m_format;
	}

	const QModelIndex DragItemData() const
	{
		return m_pDragItem;
	}


protected:

	QVariant retrieveData(const QString& mimetype, QVariant::Type preferredType) const
	{
		if (mimetype == QString::fromLatin1("ItemMimeData"))
		{
			return m_pDragItem;
		}
		else
		{
			return QMimeData::retrieveData(mimetype, preferredType);
		}
	}
private:
	/*QTreeWidgetItem*/QModelIndex   m_pDragItem;
	QStringList              m_format;
};
#endif // IMREPORTFINDINGSWIDGET_H
