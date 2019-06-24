//
//  TableNode.h
//  Elixir
//
//  Created by Peter Respondek on 3/20/15.
//
//

#ifndef __Elixir__TableNode__
#define __Elixir__TableNode__

class TableNode;
class TableNodeDelegate;
class TableSpawnDelegate;
class TableNodeSpawn;

#include "cocos2d.h"
#include "../data/TableBlock.h"
#include <set>

USING_NS_CC;

class TableSpawn
{
public:
    TableSpawn( ) :
    _next_color(NONE),
    _next_type(NORMAL),
    _delegate (nullptr),
    _pending_spawn(false){}
    
    bool popSpawn ( BlockColor& color, BlockType& type );
    
    inline BlockColor getNextColor          ( ) { return _next_color; }
    inline void setNextColor                ( BlockColor color ) { _next_color = color; }
    inline void setNextType                 ( BlockType type ){ _next_type = type; }
    inline BlockType getNextType            ( ) { return _next_type; }
    inline bool pendingSpawn                ( ) { return _pending_spawn ; }
    inline TableSpawnDelegate* getDelegate  ( ) { return _delegate; }
    inline void setDelegate                 ( TableSpawnDelegate* delegate ) { _delegate = delegate; }
    inline void setPending                  ( bool pending ) { _pending_spawn = pending; }
    
protected:
    bool                    _pending_spawn;
    BlockColor              _next_color;
    BlockType               _next_type;
    TableSpawnDelegate*     _delegate;
};

enum AxialDir {
    NORTH,
    EAST,
    SOUTH,
    WEST

};

class TableNode
{
    friend class TableGraph;
public:
    enum NodeType {
        BLOCKED,
        EMPTY,
        NO_TELEPORT,
        SPAWN,
        COLLECTER,
        SPIKES,
        PIPE
    };
    
    enum NodeState {
        ACTIVE      = 0x01,
        SHAKEN      = 0x02,
        FROZEN      = 0x04,
        CAM_TRACK   = 0x08,
        RESTRICTED  = 0x10,
        MISSLE      = 0x20,
        IMMUTABLE   = 0x40
    };
    
    enum TableDir {
        NORTH,
        NORTH_EAST,
        EAST,
        SOUTH_EAST,
        SOUTH,
        SOUTH_WEST,
        WEST,
        NORTH_WEST
    };
    
    virtual ~TableNode                      ( ) {};
    virtual void init                       ( ) {};
    void updateNode                         ( );
    static TableDir getInverseDirection     ( TableDir dir );
    virtual bool isSpawner                  ( );
    virtual bool isTile                     ( ) const { return false; }
    virtual NodeType getType                ( ) const = 0;
    virtual bool filterBlockType            ( BlockType type ) const { return isContainer(); }
    virtual bool willDiagonalFill           ( ) { return true; }
    virtual bool willDiagonalDrop           ( ) { return true; }
    virtual bool canTeleport                ( ) { return false; }
    void createSpawner                      ( uint8_t num_colors, TableBlock* block );
    TableNodeSpawn* getSpawnNode            ( );
    TableNodeSpawn* getSpawnerNode            ( );
    bool isSpawnerNode                      ( );
    bool isActive                           ( );
    bool isRestricted                       ( );
    bool isImmutable                        ( );
    void setActive                          ( bool active );
    bool isObjective                        ( );
    void purgeSpawner                       ( );
    TableSpawnDelegate* getSpawnDelegate    ( );
    void setSpawnDelegate                   ( TableSpawnDelegate* delegate );
    void setNextSpawnBlock                  ( BlockColor color, BlockType type, bool pending );
    BlockType getSpawnBlockType             ( );
    bool willSpawnBlock                     ( BlockColor color, BlockType type );
    BlockColor getSpawnBlockColor           ( );
    TableNode* getNextActiveNode            ( );
    TableNode* getPrevActiveNode            ( );
    TableNode* getPointActiveNode           ( );
    TableNode* getPrevEmptyNode             ( );
    TableNode* getPointEmptyNode            ( );
    
    virtual TableNode* getPortalNode        ( ) { return this; }
    
    virtual TableNode* getNextNode          ( std::function<bool(TableNode*)> filter = nullptr ) const;
    virtual TableNode* getPrevNode          ( std::function<bool(TableNode*)> filter = nullptr ) const;
    virtual TableNode* getPointNode         ( std::function<bool(TableNode*)> filter = nullptr ) const;

    virtual void evaluateNode               ( );
    virtual bool isContainer                ( ) const;
    virtual bool hasTouchBlock              ( );
    TableNodeDelegate* getDelegate          ( );
    void setDelegate                        ( TableNodeDelegate* delegate );
    TableBlock* getOverBlock                ( ) const;
    TableBlock* getUnderBlock               ( ) const;
    //void destroyBlock                       ( uint32_t idx );
    virtual void setOverBlock               ( TableBlock* block );
    void setUnderBlock                      ( TableBlock* block );
    void blockDestroyed                     ( TableBlock* block );
    void blockSpawned                       ( uint8_t num_colors );
    TableNode* getAdjacentActiveNode        ( TableDir dir );
    TableNode* getAdjacentNode              ( TableDir dir, std::function<bool(TableNode*)> filter = nullptr ) const;
    void getAdjacentNodes                   ( std::vector<TableNode*>& out_nodes, std::function<bool(TableNode*)> filter = nullptr ) const;
    TableDir getDirection                   ( );
    Vec2 getTablePosition                   ( );
    TableSpawn* getSpawner                  ( );
    bool isAdjacent                         ( TableNode* node );
    void shake                              ( uint32_t idx );
    void changeState                        ( NodeState state, bool set = true );
    virtual bool cavityCheck                ( );
    bool checkState                         ( NodeState state );
    
    TableNode* getDropNode                  ( const std::function<bool(TableNode*,TableBlock*)>& filter1,
                                              const std::function<bool(TableNode*,TableBlock*)>& filter2, TableBlock* block);
    TableNode* getEmptyDropNode             ( );
    TableNode* getAdjacentDropNode          ( const std::function<bool(TableNode*,TableBlock*)>& filter, TableBlock* block );
    bool isAdjacentEmptyDropNode            ( TableNode* target, TableBlock* block );
    bool isAdjacentDropNode                 ( TableNode* target, TableBlock* block );
    bool isEmptyDropNode                    ( TableNode* target, TableBlock* block );
    bool isDropNode                         ( TableNode* target, TableBlock* block );


protected:
    void clearState                         ( NodeState state );
    void setState                           ( NodeState state, bool set = true );

    // direction in which block will fall
    TableDir _node_dir = SOUTH;
    
    TableNode* _north =         nullptr;
    TableNode* _north_east =    nullptr;
    TableNode* _east =          nullptr;
    TableNode* _south_east =    nullptr;
    TableNode* _south =         nullptr;
    TableNode* _south_west =    nullptr;
    TableNode* _west =          nullptr;
    TableNode* _north_west =    nullptr;
    
    uint16_t x = 0;
    uint16_t y = 0;
    
    Vec2 force = {0,0};
    
    
    uint8_t             _node_state =   0;
    TableBlock*         _over_block =   nullptr;
    TableNodeDelegate*  _delegate =     nullptr;
    TableBlock*         _under_block =  nullptr;
    TableSpawn*         _spawner =      nullptr;
    uint8_t             _cam_idx =      255;
    uint8_t             _freeze_count = 0;
    uint32_t            _shake_idx =    0;
    
};

class TableNodeEmpty : public TableNode
{
public:
    virtual bool canTeleport () { return true; }
    virtual bool isTile () const { return true; }
    inline TableNode::NodeType getType      ( ) const
    { return TableNode::EMPTY; }
};

class TableNodeFilter : public TableNode
{
public:
    virtual bool isTile () const { return true; }
    virtual bool canTeleport () { return false; }
    inline TableNode::NodeType getType      ( ) const
    { return TableNode::EMPTY; }
    virtual bool filterBlockType            ( BlockType type ) const
    { if (!_gate || type == COIN) return true; return false; }
    virtual void evaluateNode               ( ) {
        if (getOverBlock() && filterBlockType(getOverBlock()->getType())) {
            _gate = false;
        }
        TableNode::evaluateNode();
    }

protected:
    bool _gate = true;
};

class TableNodeSpawn : public TableNode
{
public:
    TableNodeSpawn() :
    _prev_color(NONE),
    _color_count(0),
    _prev_type(NORMAL),
    _type_count(0)
    {}
    
    virtual bool willDiagonalDrop           ( ) { return false; }
    virtual bool isContainer                ( ) const
    { return true; }
    virtual bool hasTouchBlock              ( )
    { return false; }
    inline TableNode::NodeType getType      ( ) const
    { return TableNode::SPAWN; }
    virtual bool isTile () const { return true; }
    inline bool willSpawnObjective          ( ) const
    { return _objective_spawn; }
    virtual void setOverBlock               ( TableBlock* block );
    inline void setSpawnObjective           ( bool spawn )
    { _objective_spawn = spawn; }
    virtual BlockColor getNextColor ();
    virtual BlockType getNextType ()        { return NORMAL; }
    
protected:
    BlockColor              _prev_color;
    uint8_t                 _color_count;
    BlockType               _prev_type;
    uint8_t                 _type_count;
    
protected:
    bool _objective_spawn = true;
};

class TableNodeSpawnHidden : public TableNodeSpawn
{
public:
    virtual bool isTile ( ) const { return false; }
};

class TableNodeNoTeleport : public TableNodeEmpty
{
    inline TableNode::NodeType getType      ( ) const
    { return TableNode::NO_TELEPORT; }
    virtual bool canTeleport () { return false; }
    
};

class TableNodeBlocked : public TableNode
{
    virtual void evaluateNode               ( ) {}
    
    virtual bool isContainer                ( ) const
    { return false; }
    virtual bool hasTouchBlock              ( )
    { return false; }
    inline TableNode::NodeType getType      ( ) const
    { return TableNode::BLOCKED; }
};

class TableNodePipe : public TableNode
{
    friend class TableGraph;
public:
    virtual TableNode* getNextNode          ( std::function<bool(TableNode*)> filter = nullptr ) const;
    virtual TableNode* getPointNode          ( std::function<bool(TableNode*)> filter = nullptr ) const;
    virtual TableNode* getPrevNode          ( std::function<bool(TableNode*)> filter = nullptr ) const;
    virtual bool cavityCheck                ( ) { if (!getOverBlock())
        return getPointNode()->cavityCheck(); else return false; }
    virtual void evaluateNode               ( );
    virtual bool willDiagonalFill           ( ) { return false; }
    virtual bool willDiagonalDrop           ( ) { return false; }
    virtual void setOverBlock               ( TableBlock* block );
    virtual void init                       ( ) { }
    virtual bool isContainer                ( ) const
    { return false; }
    virtual bool filterBlockType            ( BlockType type ) const { return true; }
    
    virtual TableNode* getPortalNode         ( ) { return _exit->getNextNode(); }
    virtual bool hasTouchBlock              ( )
    { return false; }
    inline TableNode::NodeType getType      ( ) const
    { return TableNode::PIPE; }
    TableNode* getEntryNode                 ( ) { return _entry; }
    TableNode* getExitNode                  ( ) { return _exit; }

protected:
    TableNode* _entry = nullptr;
    TableNode* _exit = nullptr;
    

};

class TableNodeCollecter : public TableNode
{
    //virtual void evaluateNode               ( ) {}
    virtual bool isTile                     ( ) const { return false; }
    virtual bool isContainer                ( ) const { return false; }
    virtual bool willDiagonalFill           ( ) { return false; }
    virtual bool filterBlockType            ( BlockType type ) const
    { if (type == COIN) return true;
        return false; }
    virtual bool hasTouchBlock              ( )
    { return false; }
    inline TableNode::NodeType getType                  ( ) const
    { return TableNode::COLLECTER; }
};

class TableNodeSpike : public TableNode
{
    virtual bool isContainer                ( ) const { return false; }
    virtual bool willDiagonalFill           ( ) { return false; }
    virtual bool filterBlockType            ( BlockType type ) const
    { return false; }
    virtual bool hasTouchBlock              ( )
    { return false; }
    inline TableNode::NodeType getType                  ( ) const
    { return TableNode::SPIKES; }
};

class TableNodeDelegate
{
    friend class TableNode;
public:
    bool hasTouchBlock ();
    TableNode* getNode () const;
    
    // call to update node after whatever animation delay.
    void updateNode ();
    virtual void highlightNode () {};
    virtual void stateSet ( TableNode::NodeState state) {};
    
protected:
    
    
private:
    TableNode* _node = nullptr;
};

class TableSpawnDelegate
{
public:
    void spawnColorChanged ();
private:
    TableSpawn* _spawn = nullptr;
};


inline bool TableNode::checkState          ( NodeState state ) { return _node_state & state; }

inline void TableNode::clearState          ( NodeState state ) { setState(state, false); }

inline TableNode* TableNode::getAdjacentActiveNode( TableDir dir ) { return getAdjacentNode(dir, [] (TableNode* node) { if (node && node->isActive() && node->getType() != BLOCKED ) return true; return false; } ); }

inline TableNode* TableNode::getNextActiveNode ( ) { return getNextNode([] (TableNode* node) { if (node && node->isActive() && node->getType() != BLOCKED ) return true; return false; } ); }

inline TableNode* TableNode::getPrevActiveNode ( ) { return getPrevNode([] (TableNode* node) { if (node && node->isActive() && node->getType() != BLOCKED ) return true; return false; } ); }

inline TableNode* TableNode::getPointActiveNode ( ) { return getPointNode([] (TableNode* node) { if (node && node->isActive() && node->getType() != BLOCKED ) return true; return false; } ); }

inline void TableNode::createSpawner                      ( uint8_t num_colors, TableBlock* block )
{ if (!_spawner) _spawner = new TableSpawn (  );
    if (block) { _spawner->setNextColor(block->getColor());
        _spawner->setNextType(block->getType()); }
}

inline void TableNode::setActive( bool active )
{
    setState(ACTIVE, active);
}

inline bool TableNode::isActive()
{
    return checkState(ACTIVE);
}

inline bool TableNode::isRestricted()
{ return checkState(RESTRICTED); }

inline bool TableNode::isImmutable()
{ return checkState(IMMUTABLE); }

inline TableNode::TableDir TableNode::getDirection()
{ return _node_dir; }

inline Vec2 TableNode::getTablePosition()
{ return Vec2(x,y); }

inline void TableNode::purgeSpawner                       ( )
{ if (_spawner) {
    delete _spawner; _spawner = nullptr; } }

inline TableSpawnDelegate* TableNode::getSpawnDelegate     ( )
{ if (_spawner && _spawner->getDelegate()) return _spawner->getDelegate(); else return nullptr; }

inline void TableNode::setSpawnDelegate                   ( TableSpawnDelegate* delegate )
{ if (_spawner) { _spawner->setDelegate(delegate);  } }

inline void TableNode::setNextSpawnBlock(BlockColor color, BlockType type, bool pending)
{ if (_spawner) { _spawner->setNextColor(color); _spawner->setNextType(type); _spawner->setPending(pending); } }

inline BlockType TableNode::getSpawnBlockType ( )
{ if (_spawner) { return _spawner->getNextType(); } else return BlockType::NORMAL; }

inline BlockColor TableNode::getSpawnBlockColor ( )
{ if (_spawner) return _spawner->getNextColor(); else return BlockColor::NONE; }

inline bool TableNode::isObjective()
{ if (getType()==COLLECTER || getType() == SPIKES) return true; return false; }

inline TableNodeDelegate* TableNode::getDelegate   ( )
{ return _delegate; }

inline bool TableNode::willSpawnBlock(BlockColor color, BlockType type)
{
    if (_spawner && _spawner->getNextColor() == color && _spawner->getNextType() == type) return true;
    return false;
}

inline void TableNode::setDelegate                        ( TableNodeDelegate* delegate )
{ _delegate = delegate; delegate->_node = this; }

inline TableBlock* TableNode::getOverBlock         ( ) const
{ return _over_block; }

inline TableBlock* TableNode::getUnderBlock        ( ) const
{ return _under_block; }

/*inline void TableNode::destroyBlock                ( uint32_t idx )
{ if (_over_block) _over_block->destroy(idx);
  if (_under_block) _under_block->destroy(idx); }*/


inline void TableNode::blockSpawned                       ( uint8_t num_colors )
{
    //_spawner->setRandomColor( num_colors );
}

inline TableSpawn* TableNode::getSpawner()
{ return _spawner; }

#endif /* defined(__Elixir__TableNode__) */



