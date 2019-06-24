//
//  TableGraph.h
//  Elixir
//
//  Created by Peter Respondek on 1/26/15.
//
//

#ifndef __Elixir__TableGraph__
#define __Elixir__TableGraph__

class TableGraphDelegate;

#include "cocos2d.h"
#include "../data/TableBlock.h"
#include "../data/TableNode.h"
#include <array>


USING_NS_CC;

#ifndef GRAPH_DEBUG
#define GRAPH_DEBUG 0
#endif

#if !defined (GRAPH_DEBUG) || GRAPH_DEBUG == 0
#define GRAPHLOG(...)       do {} while (0)

#elif GRAPH_DEBUG == 1
#define GRAPHLOG(format, ...)      cocos2d::log(format, ##__VA_ARGS__)
#endif

struct TableMatch
{
    friend class TableGraph;
    
    enum TableMatchType {
        MATCH_NONE,
        MATCH_TWO_MIRACLE,
        MATCH_THREE,
        MATCH_FOUR_SQUARE,
        MATCH_FOUR_VERTICAL,
        MATCH_FOUR_HORIZONTAL,
        MATCH_FIVE_T,
        MATCH_FIVE_L,
        MATCH_FIVE_LINE,
        MATCH_SEVEN,
        MATCH_TWO_MISSLE_VERTICAL,
        MATCH_TWO_MISSLE_HORIZONTAL,
        MATCH_TWO_MISSLE_BLAST,
        MATCH_TWO_MISSLE_CROSS,
        MATCH_TWO_MISSLE_MISSLE,
        MATCH_TWO_BLAST_VERTICAL,
        MATCH_TWO_BLAST_HORIZONTAL,
        MATCH_TWO_X,
        MATCH_TWO_X_BIG,
        MATCH_TWO_BLAST_BIG,
        MATCH_TWO_STAR,
        MATCH_TWO_CROSS,
        MATCH_TWO_MANA,
        MATCH_OMNI,
        MATCH_TWO_OMNI
    } match_type = MATCH_NONE;
    bool isMatchTwo() {
        switch(match_type) {
            case MATCH_TWO_MISSLE_VERTICAL:
            case MATCH_TWO_MISSLE_HORIZONTAL:
            case MATCH_TWO_MISSLE_BLAST:
            case MATCH_TWO_MISSLE_CROSS:
            case MATCH_TWO_MISSLE_MISSLE:
            case MATCH_TWO_X:
            case MATCH_TWO_BLAST_VERTICAL:
            case MATCH_TWO_BLAST_HORIZONTAL:
            case MATCH_TWO_X_BIG:
            case MATCH_TWO_BLAST_BIG:
            case MATCH_TWO_STAR:
            case MATCH_TWO_CROSS:
            case MATCH_TWO_MIRACLE:
            case MATCH_TWO_MANA:
            case MATCH_OMNI:
                return true;
                break;
            default:
                break;
        }
        return false;
    }
    bool isHint() {
        switch(match_type) {
            /*case MATCH_TWO_MIRACLE:
                return false;*/
            default:
                break;
        }
        return true;
    }
    bool willDestroyUnderBlocks() {
        switch(match_type) {
            case MATCH_TWO_MIRACLE:
                return false;
            default:
                break;
        }
        return true;
    }
    bool in (TableNode* node) {
        return std::find(nodes.begin(), nodes.end(), node) != nodes.end();
    }
    bool in (TableBlock* block) {
        return std::find(blocks.begin(), blocks.end(), block) != blocks.end();
    }
    BlockColor source_color =   BlockColor::NONE;
    BlockColor target_color =   BlockColor::NONE;
    BlockType type =            BlockType::NORMAL;
    std::vector<TableNode*>     nodes;
    std::vector<TableBlock*>    blocks;
    uint32_t                    id;
private:
    TableMatch() {}
};

struct AxialMatch {
    std::vector<TableNode*> vertical_matches;
    std::vector<TableNode*> horizontal_matches;
    std::vector<TableNode*> north_matches;
    std::vector<TableNode*> south_matches;
    std::vector<TableNode*> east_matches;
    std::vector<TableNode*> west_matches;
    
    bool crossPattern () {
        return (north_matches.size() == 1 && south_matches.size() == 1 && east_matches.size() == 1 && west_matches.size() == 1);
    }

};

struct TableHint
{
public:
    ~TableHint() { if (match) delete match; }
    TableNode* source;
    TableNode::TableDir dir;
    uint8_t priority = 0;
    TableMatch* match;
    
    static bool compare (const TableHint* lhs, const TableHint* rhs)
    {
        if (lhs->priority != rhs->priority) return lhs->priority < rhs->priority;
        return int(lhs->match->match_type) < int(rhs->match->match_type);
    }
};

class TableGraph : public Ref
{
    friend class TableNode;
public:
    
    enum TableState {
        READY,
        ACTIVE
    };
    
    enum ActiveState {
        ALL,
        ACTIVE_ONLY,
        INACTIVE_ONLY
    };
    
    static TableGraph* getInstance              ( );
    static TableGraph* createWithValueMap       ( ValueMap& map );
    
    TableMatch* createMatch                     ( );
    
    void setDelegate                            ( TableGraphDelegate* delegate );
    void blocksWithValueMap                     ( ValueVector& color, BlockChannel channel );
    void spawnBlocksWithValueMap                ( ValueVector& color );


    Vec2 getDimensions                          ( );
    
    void getNodesByRegion                       ( Rect& region, std::vector<TableNode*>& nodes);
    const std::vector<TableNode*>& getNodes     ( );
    //void getNodesByType     ( TableNodeType type, std::vector<TableNode*>& out_nodes );
    const std::set<TableBlock*>& getOverBlocks  ( );
    const std::set<TableBlock*>& getUnderBlocks ( );
    void getSpawners                            ( std::vector<TableNode*>& out_spawners, uint8_t opt = 0 );
    bool hasSpawners                            ( );
    //void blockSpawned                           ( TableSpawn* spawn );


    
    // call when moving block. Will return true if matches are found. Matchs will be destroyed.
    bool blockMoved                             ( TableBlock* source, TableBlock* destination, bool match_two = true );
    // check for matches when a block is inserted.
    bool blockAdded                             ( TableBlock* source );
    // checks if source node moved to destination will results in match three.
    bool willMatchThree                         ( TableBlock* source, TableBlock* destination );
    // update is where most of the action happens.
    //void evaluateNode                           ( TableNode* node );
    TableState  getState                        ( ) { return _table_state; }
    void increaseFallCount                      ( );
    void decreaseFallCount                      ( );
    
    bool requestHint                            ( BlockColor color = NONE );
    TableHint* createHint                       ( BlockColor color = NONE );

    // actually destroy blocks and remove from node
    void resetBlock                             ( TableBlock* block );
    void destroyBlock                           ( TableBlock* block, uint32_t idx );
    void spawnBlock                             ( TableNode* node );
    TableBlock* insertBlock                     ( TableNode* node, BlockColor color, BlockType type );
    TableBlock* replaceBlock                    ( TableNode* node, BlockColor color, BlockType type );
    
    /** insert a block in node. This will check if node is already filled and try to find alternative if possile */
    void insertBlock                            ( TableNode* node, TableBlock* block );
    void replaceBlock                           ( TableNode* node, TableBlock* block );
    
    void stopBlock                              ( TableNode* node );
    //void spawnBlock                             ( TableNode* node );

    bool spawnBlockAtRandomNode                 ( BlockColor color, BlockType type );
    bool hasSpawnBlock                          ( BlockColor color, BlockType type );

    /* silently erase block without disturbing surrounding blocks */
    void eraseBlock                             ( TableBlock* block );
    void shakeBlock                             ( TableBlock* block, TableNode* node, uint32_t idx );
    
    /** will destroy block and's node underblock. block may not be node's overblock. Can shake blocks adjacent to node. */
    void destructBlocks                         ( std::vector<TableBlock*>* blocks );
    /**
     * Destroy a block in Node.
     *
     * @param   block to be destroyed.
     * @param   node the block is in.
     * @param   shake surrounding blocks
     * @param   immunity index. index of zero will be destroyed regardless.
     * @param   whether a new immunity index should be generated.
     */
    void destructBlock                          ( TableBlock* block, TableNode* node, bool shake, uint32_t idx);

    void setMatchState                          ( TableMatch* match, FallState state );
    void setBlockState                          ( TableBlock* block, FallState state );
    void setMatchType                           ( TableMatch* match, BlockType type );
    void blockFallStateChanged                  ( TableBlock* block, FallState state, uint8_t inc );
    void nodeStateChanged                       ( TableNode* node, TableNode::NodeState state, bool set, uint8_t inc );
    
    void evaluateBorderNodes                    ( std::vector<TableNode*>& block_nodes );
    void evaluateBorderNodes                    ( TableNode* node );
    void evaluateNodes                          ( );
    
    void destroyMatch                           ( TableMatch* match );
    void shuffleBlock                           ( TableBlock* block );
    void clearMatches                           ( ActiveState active );
    void shakeAdjacentNodes                     ( std::vector<TableNode*>& block_nodes, uint32_t idx );
    
    void prepareDelegate                        ( );
    uint32_t incrementMatchCounter              ( );
    void checkGraph                             ( );
    void printGraph                             ( );

    void swapBlocks                             ( TableNode* t1, TableNode* t2 );
    void setFruitColor                          ( BlockColor color );
    BlockColor getFruitColor                    ( );
    void getAdjacentNodes                       ( TableNode* node, std::vector<TableNode*>& adj_nodes, bool active = true );
    void getBorderNodes                         ( TableNode* node, std::vector<TableNode*>& adj_nodes, bool active = true );
    void dispatchMatches                        ( TableMatch* match );
    void freezeNode                             ( TableNode* node );
    void replaceBlockType                       ( BlockType old_type, BlockType new_type, ActiveState active );

    /* iterate through blocks and return whether a block meet conditions*/
    uint16_t hasOverBlock                       ( BlockColor color,
                                                 BlockType type ,
                                                 FallState state, bool prisoners = true);
    // camera helper functions
    TableNode* calcCurrentTrackNode             ( );
    TableNode* calcCurrentJumpNode              ( );
    void       calcTrackNodes                   ( );
    void       calcJumpNodes                    ( );

    // get the current camare track beginning based on the group set in
    TableNode* getTrackStartNode                ( );
    
    TableNode* getTrackEndNode                  ( );

    // find the camera track node closest to the input node;
    TableNode* findTrackNode                    ( TableNode* node );
    // get Node based on x,y position in table
    TableNode* getNodeByPosition                ( Vec2 position );
    // returns the distance from the current track node to its root
    uint16_t findTrackEnd                       ( TableNode* in, TableNode** out, bool forward = false );
    // move down camera track based on distance
    TableNode* advanceTradeNode                 ( TableNode* node, int16_t dist );
    // returns the number of camera groups
    uint8_t getNumCamGroup                      ( ) const;
    uint8_t getCamGroup                         ( ) const;
    // set the camera group. Further calls will result in this group being queried.
    void setCamGroup                            ( uint8_t idx );
    TableNode* getCamNode                       ( uint8_t idx );
        
    void filterBlocks( std::vector<TableBlock*>* out_blocks,
                       const std::function<bool(TableNode*)>& node_condition,
                       const std::function<bool(TableBlock*)>& block_condition,
                       const std::function<void(TableBlock*)>& operation );
    void filterNodes( std::vector<TableNode*>* out_nodes,
                      const std::function<bool(TableNode*)>& node_condition,
                      const std::function<bool(TableBlock*)>& block_condition,
                      const std::function<void(TableNode*)>& operation );
    void getShuffleFilter( std::function<bool(TableNode*)>& node_func,
                          std::function<bool(TableBlock*)>& block_func,
                          ActiveState state );
    bool nodeActive(TableNode* node);
    bool nodeInactive(TableNode* node);
    bool shuffleBlocks( std::vector<TableBlock*>* out_blocks, ActiveState active );
    
    bool canDispelBlock(TableBlock *block);
    bool canTeleportBlock(TableBlock *block);
    bool isNormalBlock(TableBlock *block);
    bool canShuffleBlock(TableBlock *block);
    bool canSwapBlock(TableBlock *block);
    bool canHammerBlock(TableBlock *block);
    bool canPaintBlock(TableBlock *block);
    
    void getIsland ( std::vector<TableNode*>& out_nodes,
                     TableNode* node,
                     const std::function<bool(TableNode*)>& node_func );
    void getBoundryNodes ( std::vector<TableNode*>& out_nodes,
                           std::vector<TableNode*>& in_nodes,
                           const std::function<bool(TableNode*)>& node_func );
    
    void checkForDirectionHint                  ( TableNode* node, std::vector<TableHint*>& hints );
    TableMatch* checkForMatchs                  ( TableNode* node, bool falling, BlockColor color = NONE,
                                                 bool active = true);
protected:
    TableGraph                                  ( );
    virtual ~TableGraph                         ( );
    
    /** create or move a block delegate to specified node. This is helpful when making a block that is, for instance, falling
     from another node */
    void blockDelegateToNode                    ( TableBlock* block, TableNode* node );
    /** emplace a block in node. Will overwrite block in node. */
    void emplaceBlock                           ( TableNode* node, TableBlock* block );
    /** replace block with block but keep all other values. */
    void replaceBlock                           ( TableBlock* source, TableBlock* target );
    /** drop block into node. Handles delegate movment */
    void dropBlock                              ( TableBlock* block, TableNode* source, TableNode* target );
    TableBlock*   spawnBlock                    ( TableNode *node, TableBlock *source );
    void  spawnBlockChanged                     ( TableBlock* block, TableNode* node );

    void calcHintPriority                       ( TableHint* hint );
    void clearBlock                             ( TableBlock* block );
    void clearMatch                             ( TableMatch* match );
    void removeBlock                            ( TableBlock* block, uint32_t idx, bool silent = false );
    void removeMatch                            ( TableMatch* match );
    void checkForColorHint                      ( TableNode* node, std::vector<TableHint*>& hints, BlockColor color );
    
    void checkForMatchTwoType                   ( TableBlock* source, TableBlock* destination,
                                                  TableMatch::TableMatchType& out_match_type, BlockType& out_block_type );
    TableMatch* checkForMatchTwo                ( TableBlock* source, TableBlock* destination );
    
    void checkForAxialMatchs                    ( TableNode* node, BlockColor color, AxialMatch& out_match, bool falling, bool active );
    TableNode* tTrimAxialMatchs                 ( AxialMatch& out_match );
    void lTrimAxialMatchs                       ( AxialMatch& out_match );

    TableBlock* createTableBlock                ( BlockColor color = NONE, BlockType type = NORMAL );

    
    // set block state.
    //void setBlockState                          ( TableBlock* node, TableBlock::BlockState state );
    //void setBlockType                           ( TableBlock* node, BlockType type );
    void setNodeState                           ( TableNode* node, TableNode::NodeState state );
       
    void checkAdjColors                         ( TableNode* node, std::set<BlockColor>& colors);
    void checkAdjDir                            ( TableNode* node, BlockColor color, TableNode::TableDir dir, std::vector<TableNode*>& matches, bool falling = false, bool active = true );
    static bool compareHints                    ( TableHint* lhs, TableHint* rhs );
    
    bool initWithValueMap                       ( ValueMap& map );
    

private:
    uint8_t                                         _cam_group;
    TableNode*                                      _cam_nodes [9];
    BlockColor                                      _fruit_color;
    Vec2                                            _graph_dim;
    //Vec2                                            _graph_offset;
    std::set<TableBlock*>                           _over_blocks;
    std::set<TableBlock*>                           _under_blocks;
    std::vector<TableNode*>                         _nodes;
    TableGraphDelegate*                             _delegate;
    uint32_t                                        _fall_count;
    TableState                                      _table_state;
    uint32_t                                        _match_counter;
    
    std::map<std::string,std::vector<std::string>>        _graph_debug;


};

// This class is used to inform the front-end of the graphs intent to preform some action. The front-end must then
// inform the graph when it is ready for the action to be performed. This allows for a delay while effects and
// animations are played.

class TableGraphDelegate
{
    friend class TableGraph;
public:
    
    //virtual void horizontalBlast            ( TableNode* source ) = 0;
    // called to show blocks hint
    virtual void blockFrozen                ( TableBlock* block, bool frozen ) = 0;
    virtual void blocksHint                 ( TableHint* hint ) = 0;
    virtual void blockStopped               ( TableBlock* block ) = 0;
    virtual void blockErased                ( TableBlock* block ) = 0;
    virtual void blockReset                 ( TableBlock* block ) = 0;
    virtual void blockShake                 ( TableBlock* block, TableNode* node, uint32_t idx ) = 0;
    // called when block matches are found. First block in the source of the match.
    virtual void blocksMatch                ( TableMatch* match ) = 0;
    virtual void matchDestroyed             ( TableMatch* match ) = 0;
    virtual void blockRemoved               ( TableBlock* block, uint32_t idx ) = 0;
    // called when blocks are destroyed by something other than a match. Usually from blocks exploding.
    virtual void blocksDestroyed            ( std::vector<TableNode*>* pending_nodes, BlockType source ) = 0;
    virtual void blockDestroyed             ( TableBlock* node ) = 0;
    // called when player swaps a block.
    virtual void blockMoveToTarget          ( TableBlockDelegate* block, TableNodeDelegate* source, TableNodeDelegate* target ) = 0;
    virtual TableBlockDelegate* blockCreated( TableBlock* block, TableNode* node, bool under = false ) = 0;
    virtual TableBlockDelegate* blockSpawned( TableBlock* block, TableNode* node, TableNode::TableDir dir ) = 0;
    virtual void blockInserted              ( TableBlock* block, TableNode* node, bool under = false ) = 0;
    virtual void graphReady                 ( ) = 0;
    virtual void graphActive                ( ) = 0;
    virtual void underBlockDestroyed        ( TableNode* node ) = 0;
    virtual void spawnColorChanged          ( TableNode* node ) = 0;
    virtual TableNode* nodeInBlock          ( TableBlock* block ) = 0;
    virtual TableBlock* blockInNode         ( TableNode* node ) = 0;
    virtual void spawningBlock              ( TableNode* node ) = 0;


};

inline uint8_t TableBlockDelegate::getFallCount() { return _block->_fall_count; }
inline TableNode* TableGraph::getNodeByPosition(cocos2d::Vec2 position)
{
    int pos = (position.y * _graph_dim.x) + position.x;
    if (pos < _nodes.size()) return _nodes.at(pos);
    return nullptr;
}

inline uint8_t TableGraph::getNumCamGroup() const
{
    for (uint8_t i = 0; i < 9; i++) {
        if ( _cam_nodes[i] == nullptr ) return i;
    }
    return 0;
}

inline TableNode* TableGraph::getCamNode(uint8_t idx)
{
    return _cam_nodes[idx];
}

inline void TableGraph::setCamGroup(uint8_t idx)
{
    CCASSERT(idx < 9, "max camera groups is 9");
    if (idx < 9)
        _cam_group = idx;
}

inline uint8_t TableGraph::getCamGroup() const
{ return _cam_group; }
inline BlockColor TableGraph::getFruitColor()
{ return _fruit_color; }

#endif /* defined(__Elixir__TableGraph__) */
