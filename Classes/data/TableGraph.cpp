//
//  TableGraph.cpp
//  Elixir
//
//  Created by Peter Respondek on 1/26/15.
//
//

#include "../data/TableGraph.h"
#include <unordered_set>
#include <stdlib.h>


static TableGraph* s_graph = nullptr;


TableGraph::TableGraph( ) :
_fall_count(0),
_table_state(READY),
_graph_dim(0,0),
_delegate(nullptr),
_fruit_color(NONE),
_cam_group(0),
_match_counter(0)
{
    for (uint8_t i = 0; i < sizeof(_cam_nodes) / sizeof(TableNode*); i++) {
        _cam_nodes[i] = nullptr;
    }
}


TableGraph::~TableGraph( )
{
    for (auto i : _nodes) {
        if (i != nullptr)
            delete i;
    }
    for (auto i : _over_blocks) {
        if (i != nullptr)
            delete i;
    }
    for (auto i : _under_blocks) {
        if (i != nullptr)
            delete i;
    }
    _over_blocks.clear();
    _under_blocks.clear();
    _nodes.clear();
    s_graph = nullptr;
}

TableGraph* TableGraph::getInstance ( )
{
    CCASSERT(s_graph, "graph not initialised");
    return s_graph;
}

TableGraph* TableGraph::createWithValueMap (ValueMap& layout)
{
    if (s_graph) CC_SAFE_DELETE(s_graph);
    s_graph = new TableGraph();
    if (s_graph && s_graph->initWithValueMap(layout)) {
        s_graph->autorelease();
        return s_graph;
    }
    CC_SAFE_DELETE(s_graph);
    return nullptr;
}

bool TableGraph::initWithValueMap(ValueMap& layout)
{
    ValueVector s =                 ValueVector();
    ValueVector& hint = s;
    ValueVector& level =            layout.at("Nodes").asValueVector();
    ValueVector& direction =        layout.at("Direction").asValueVector();
    ValueVector& camera_group =     layout.at("CameraGroup").asValueVector();
    ValueVector& camera_track =     layout.at("CameraTrack").asValueVector();
    TableBlock::num_block_colors =  layout.at("BlockColors").asInt();
    auto iter = layout.find("Hints");
    if (iter != layout.end()) {
        hint = (*iter).second.asValueVector();
    } else {
        hint.insert(hint.end(),level.size(), Value(std::string(level.begin()->asString().size(),'0')));
    }
    
    CCASSERT(TableBlock::num_block_colors > 2 &&
             TableBlock::num_block_colors < 7, "Block colors out of range");

    _graph_dim.x = level.front().asString().length();
    _graph_dim.y = level.size();
    
    std::map<uint8_t,std::pair<TableNodePipe*,TableNodePipe*>> pipes;

    int y = 0;
    auto k = direction.rbegin();
    auto c = camera_group.rbegin();
    auto t = camera_track.rbegin();
    auto h = hint.rbegin();

    for (auto i = level.rbegin(); i != level.rend(); i++) {
        int x = 0;
        std::string row_str = i->asString();
        std::string dir_str = k->asString();
        std::string cam_str = c->asString();
        std::string tra_str = t->asString();
        std::string hin_str = h->asString();


        CCASSERT(row_str.length() == _graph_dim.x, "node values out of range");
        auto m = dir_str.begin();
        auto d = cam_str.begin();
        auto r = tra_str.begin();
        auto q = hin_str.begin();
        for (auto j : row_str ) {
            char node_hint = *q;
            char node_type = j;
            char node_dir = *m;
            char cam_idx = *d;
            char track = *r;
            bool restricted = false;
            bool immutable = false;
            
            TableNode* node;
            switch (node_type) {
                case 'H':
                    node = new TableNodeSpawnHidden();
                    break;
                case 'h': {
                    auto snode = new TableNodeSpawnHidden();
                    snode->setSpawnObjective(false);
                    node = snode;
                }
                    break;
                case 'S':
                    node = new TableNodeSpawn();
                    break;
                case 'r':
                    node = new TableNodeSpawn();
                    restricted = true;
                    immutable = true;
                    break;
                case 's': {
                    TableNodeSpawn* snode = new TableNodeSpawn();
                    snode->setSpawnObjective(false);
                    node = snode;
                }
                    break;
                case 'R':
                    restricted = true;
                case 'I':
                    immutable = true;
                case '0':
                    node = new TableNodeEmpty();
                    break;
                case 'X':
                    node = new TableNodeBlocked();
                    break;
                case 'F':
                    node = new TableNodeFilter();
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case ':':
                case ';':
                case '<':
                case '=':
                case '>':
                case '?':
                case '@':
                case 'A':
                case 'B':
                {
                    TableNodePipe* pipe = new TableNodePipe();
                    uint8_t idx = node_type - 49;
                    if (idx > 8) idx -= 9;
                    auto iter = pipes.find(idx);
                    if (iter == pipes.end()) {
                        iter = pipes.insert(std::pair<uint8_t,std::pair<TableNodePipe*,TableNodePipe*>>(idx,std::pair<TableNodePipe*,TableNodePipe*>(nullptr,nullptr))).first;
                    }
                    if ( node_type - 49 > 8 ) {
                        iter->second.second = pipe;
                    } else {
                        iter->second.first = pipe;
                    }
                    node = pipe;
                }
                    break;
                case 'P':
                    node = new TableNodePipe();
                    break;
                case 't':
                    restricted = true;
                case 'T':
                    node = new TableNodeNoTeleport();
                    break;
                case 'C':
                    node = new TableNodeCollecter();
                    break;
                case '^':
                    node = new TableNodeSpike();
                    break;
                default:
                    CCASSERT(false, "Node character not recognised");
                    break;
            }
            switch (node_hint) {
                case '1':
                    node->setState(TableNode::NodeState::MISSLE);
                    break;
                default:
                    break;
            }
            switch (node_dir) {
                case 'v':
                    node->_node_dir = TableNode::SOUTH;
                    break;
                case '<':
                    node->_node_dir = TableNode::WEST;
                    break;
                case '>':
                    node->_node_dir = TableNode::EAST;
                    break;
                case '^':
                    node->_node_dir = TableNode::NORTH;
                    break;
                default:
                    CCASSERT(false, "Node direction not recognised");
                    break;
            }
            switch (track) {
                case '0':
                    node->setState(TableNode::CAM_TRACK, true);
                    break;
                default:
                    break;
            }
            if (restricted == true) {
                node->setState(TableNode::RESTRICTED);
            }
            if (immutable == true) {
                node->setState(TableNode::IMMUTABLE);
            }
            node->_cam_idx = cam_idx - 48;
            node->x = x;
            node->y = y;
            _nodes.push_back(node);
            x++;
            m++;
            d++;
            r++;
            q++;
        }
        k++;
        y++;
        c++;
        t++;
        h++;
    }
    
    // make adjacency graph edges
    for (uint32_t y = 0; y < _graph_dim.y; y++) {
        for (uint32_t x = 0; x < _graph_dim.x; x++) {
            TableNode* node = _nodes.at((_graph_dim.x * y) + x);
            if (y < _graph_dim.y - 1) {
                node->_north = _nodes.at((_graph_dim.x * (y + 1)) + x);
            }
            if (y < _graph_dim.y - 1 && x < _graph_dim.x - 1) {
                node->_north_east = _nodes.at((_graph_dim.x * (y + 1)) + x + 1);
            }
            if (x < _graph_dim.x - 1) {
                node->_east = _nodes.at((_graph_dim.x * y) + x + 1);
            }
            if (y > 0 && x < _graph_dim.x - 1) {
                node->_south_east = _nodes.at((_graph_dim.x * (y - 1)) + x + 1);
            }
            if (y > 0) {
                node->_south = _nodes.at((_graph_dim.x * (y - 1)) + x);
            }
            if (y > 0 && x > 0) {
                node->_south_west = _nodes.at((_graph_dim.x * (y - 1)) + x - 1);
            }
            if (x > 0) {
                node->_west = _nodes.at((_graph_dim.x * y) + x - 1);
            }
            if (y < _graph_dim.y - 1 && x > 0) {
                node->_north_west = _nodes.at((_graph_dim.x * (y + 1)) + x - 1);
            }
        }
    }
    
    for (auto& pipe : pipes) {
        TableNodePipe* lhs = pipe.second.first;
        TableNodePipe* rhs = pipe.second.second;
        if (rhs == nullptr) rhs = lhs;
        rhs->_exit = rhs; lhs->_exit = rhs;
        lhs->_entry = lhs; rhs->_entry = lhs;
    }
    
    for (auto node : _nodes) {
        node->init();
    }


    return true;
}

void TableGraph::calcTrackNodes()
{
    // Trace back to start of each camera track and store the root node.
    // This is a convenience so we can grab them as needed.
    for (auto node : _nodes) {
        if (node->checkState(TableNode::CAM_TRACK) && node->_cam_idx < 10 && _cam_nodes[node->_cam_idx] == nullptr)  {
            TableNode* root_node = nullptr;
            findTrackEnd(node, &root_node);
            _cam_nodes[node->_cam_idx] = root_node;
        }
    }
    
    CCASSERT(_cam_nodes[0], "Default camera node not assigned");
}

void TableGraph::calcJumpNodes()
{
    for (auto node : _nodes) {
        if (node->checkState(TableNode::CAM_TRACK) && node->_cam_idx < 10 && _cam_nodes[node->_cam_idx] == nullptr)  {
            _cam_nodes[node->_cam_idx] = node;
        }
    }
    
    CCASSERT(_cam_nodes[0], "Default camera node not assigned");
}



uint16_t TableGraph::findTrackEnd (TableNode* in, TableNode** out, bool forward)
{
    CCASSERT(in->checkState(TableNode::CAM_TRACK), "node is not a camera track");
    if (!in->checkState(TableNode::CAM_TRACK)) in = findTrackNode(in);
    TableNode* cam_node = in;
    uint16_t counter = 0;
    while (true) {
        TableNode* prev_node = nullptr;
        if (forward) prev_node = cam_node->getNextNode();
        else prev_node = cam_node->getPointEmptyNode();
        if (!prev_node || prev_node->checkState(TableNode::CAM_TRACK) != in->checkState(TableNode::CAM_TRACK) || prev_node->_cam_idx != in->_cam_idx)
            break;
        counter++;
        cam_node = prev_node;
    }
    if (out) *out = cam_node;
    return counter;
}

TableNode* TableGraph::advanceTradeNode                 (TableNode* node, int16_t dist)
{
    if (!node->checkState(TableNode::CAM_TRACK)) node = findTrackNode(node);
    if (dist == 0) return node;
    for (uint8_t i = 0; i < abs(dist); i++) {
        TableNode* next_node = nullptr;
        if (dist > 0) next_node = node->getNextNode();
        else next_node = node->getPointEmptyNode();
        if (!next_node->checkState(TableNode::CAM_TRACK) || next_node->getDirection() != node->getDirection()) return node;
        node = next_node;
    }
    return node;
}


void TableGraph::spawnBlocksWithValueMap                ( ValueVector& drop_blocks )
{
    std::vector<TableNode*> spawners;
    getSpawners(spawners);
    for (auto i = drop_blocks.rbegin(); i != drop_blocks.rend(); i++) {
        std::string row_str = i->asString();
        uint16_t counter = 0;
        for (auto j = row_str.begin(); j != row_str.end(); j++) {
            if (counter > spawners.size()-1) {
                GRAPHLOG("%s", "Not enought spawners to fill");
                break;
            }
            
            TableSpawn* spawn = spawners.at(counter)->getSpawner();
            //spawn->appendBlockSpawnList(*j);
            counter++;
        }
    }
    /*for (auto i : spawners ) {
        i->getSpawner()->setRandomColor(TableBlock::num_block_colors);
    }*/
}

void TableGraph::blocksWithValueMap( ValueVector &block_map, BlockChannel channel )
{
    uint32_t counter = 0;

    for (auto i = block_map.rbegin(); i != block_map.rend(); i++) {
        std::string row_str = i->asString();
        CCASSERT(row_str.length() == _graph_dim.x, "node values out of range");
        for (auto j = row_str.begin(); j != row_str.end(); j++) {
            
            // nodes that not empty can't contain block so skip over.
            TableNode* node = _nodes.at(counter);
            counter++;
            if (!node->isContainer()) {
                continue;
            }
            
            // assign our block based on string char
            char block_type = *j;
            
            switch (channel) {
                case UNDERBLOCK: {
                    TableBlock* block = TableBlock::createBlockWithChar(block_type, channel);
                    if (block) {
                        node->setUnderBlock(block);
                        _under_blocks.insert(block);
                    }
                }
                    break;
                case OVERBLOCK_TYPE: {
                    TableBlock* block = TableBlock::createBlockWithChar(block_type, channel);
                    if (block) {
                        node->setOverBlock(block);
                        _over_blocks.insert(block);
                    }
                }
                    break;
                case OVERBLOCK_COLOR: {
                    TableBlock* block = node->getOverBlock();
                    if (block) {
                        if (block->getType() == MANA) {
                            static_cast<TableBlockMana*>(block)->setSourceColor(TableBlock::getBlockColorWithChar(block_type));
                        } else {
                            block->setBlockColor(TableBlock::getBlockColorWithChar(block_type));
                        }
                        if (block_type != 'X') block->setState(IMMUTABLE, true);
                    }
                }
                    break;
                case MUTATOR: {
                    TableBlock* block = TableBlock::createMutatorWithChar(block_type, node->getOverBlock());
                    if (block) {
                        node->setOverBlock(block);
                        _over_blocks.insert(block);
                    }
                }
                    break;
            }
            
        }
    }
}

void TableGraph::freezeNode ( TableNode* node )
{
    TableBlock* block = TableBlock::createMutatorWithChar('F', node->getOverBlock());
    _delegate->blockErased(node->getOverBlock());
    node->setOverBlock(block);
    _over_blocks.insert(block);
    block->setDelegate(_delegate->blockCreated(block, node, false));
    _delegate->blockFrozen(block, false);
    block->getDelegate()->freeze();
}
void TableGraph::filterNodes( std::vector<TableNode*>* out_nodes,
                              const std::function<bool(TableNode*)>& node_condition,
                              const std::function<bool(TableBlock*)>& block_condition,
                              const std::function<void(TableNode*)>& operation )
{
    const std::vector<TableNode*>& nodes = getNodes();
    
    for ( auto i : nodes ) {
        if (( node_condition && !node_condition(i) ) ||
            ( block_condition && !block_condition( i->getOverBlock( ) )) )
            continue;
        if ( out_nodes ) out_nodes->push_back( i );
        if ( operation ) operation( i );
    }
}

void TableGraph::filterBlocks( std::vector<TableBlock*>* out_blocks,
                               const std::function<bool(TableNode*)>& node_condition,
                               const std::function<bool(TableBlock*)>& block_condition,
                               const std::function<void(TableBlock*)>& operation )
{
    const std::vector<TableNode*>& nodes = getNodes();
    
    for ( auto i : nodes ) {
        if (( node_condition && !node_condition(i) ) ||
            ( block_condition && !block_condition( i->getOverBlock( ) )) )
            continue;
        if ( out_blocks ) out_blocks->push_back( i->getOverBlock( ) );
        if ( operation ) operation( i->getOverBlock( ) );
    }
}

bool TableGraph::shuffleBlocks( std::vector<TableBlock*>* out_blocks, ActiveState active )
{
    std::function<bool(TableNode*)> node_func = nullptr;
    std::function<bool(TableBlock*)> block_func = nullptr;
    getShuffleFilter ( node_func, block_func, active );
    uint8_t counter = 0;
    bool shuffled = false;
    do {
        if (out_blocks)
            out_blocks->clear();
        filterBlocks( out_blocks, node_func,
                       block_func,
                       CC_CALLBACK_1(TableGraph::shuffleBlock,this) );
        clearMatches(active);
        shuffled = requestHint();
        counter++;
    } while ( !shuffled && counter < 10);
    return shuffled;
}

void TableGraph::getShuffleFilter( std::function<bool(TableNode*)>& node_func,
                                   std::function<bool(TableBlock*)>& block_func,
                                   ActiveState state )
{
    switch ( state ) {
        case ACTIVE_ONLY:
            node_func = CC_CALLBACK_1(TableGraph::nodeActive,this);
            block_func = CC_CALLBACK_1(TableGraph::canShuffleBlock,this);
            break;
        case INACTIVE_ONLY:
            node_func = CC_CALLBACK_1(TableGraph::nodeInactive,this);
            block_func = CC_CALLBACK_1(TableGraph::canShuffleBlock,this);
            break;
        case ALL:
            node_func = [this] (TableNode* node) -> bool { return true; };
            block_func = [this] (TableBlock* block) -> bool {
                if (block && !block->checkState(IMMUTABLE) && block->getColor() < num_block_colors ) return true;
                return false;
            };
            break;
    }
}

uint32_t TableGraph::incrementMatchCounter()
{
    _match_counter++;
    return _match_counter;
}

void TableGraph::prepareDelegate()
{
    for (auto i : _nodes) {
        if (i->getOverBlock()) {
            i->getOverBlock()->setDelegate(_delegate->blockCreated(i->getOverBlock(), i, false));
            if (i->getOverBlock()->getType() == NORMAL || i->getOverBlock()->getType() == FRUIT) {
                i->getOverBlock()->setState(IMMUTABLE, false);
            }
        }
        if (i->getUnderBlock())
            i->getUnderBlock()->setDelegate(_delegate->blockCreated(i->getUnderBlock(), i, true));
    }
}

bool TableGraph::nodeActive(TableNode* node)
{
    if (node->isActive()) return true;
    return false;
}

bool TableGraph::nodeInactive(TableNode* node)
{
    if (!node->isActive()) return true;
    return false;
}

bool TableGraph::canDispelBlock(TableBlock *block)
{
    if (block && block->getType() == EYE)
        return true;
    return false;
}

bool TableGraph::canTeleportBlock(TableBlock *block)
{
    if (block && block->getFallState() != STATIC && block->getNode()->canTeleport() && block->getNode())
        return true;
    return false;
}

bool TableGraph::isNormalBlock(TableBlock *block)
{
    if ( block ) {
        auto type = block->getType();
        if ((type == NORMAL || type == FRUIT) &&
        block->getFallState() != STATIC &&
        !block->checkState(IMMUTABLE) &&
        !block->isPetrified())
            return true;
    }
    return false;
}

bool TableGraph::canShuffleBlock(TableBlock *block)
{
    return ( isNormalBlock(block) && !block->getNode()->isImmutable());
}

bool TableGraph::canSwapBlock(TableBlock *block)
{
    return ( block && block->getFallState() != STATIC && !block->isPetrified() && block->getNode());
}

void TableGraph::getIsland ( std::vector<TableNode*>& out_nodes, TableNode* node, const std::function<bool(TableNode*)>& node_func )
{
    std::set<TableNode*> nodes;
    
    std::function<void( std::set<TableNode*>&, TableNode* )> func =
    [this, &func, &node_func] ( std::set<TableNode*>& nodes, TableNode* node ) {
        std::vector<TableNode*> border_nodes;
        getBorderNodes( node, border_nodes, true );
        for ( auto nnode : border_nodes ) {
            if ( nnode && node_func(nnode) ) {
                auto iter = nodes.insert(nnode);
                if (iter.second) {
                    func (nodes, nnode);
                }
            }
        }
    };
    func(nodes, node);
    for (auto nnode : nodes) {
        out_nodes.push_back(nnode);
    }
}

void TableGraph::getBoundryNodes ( std::vector<TableNode*>& out_nodes,
                                   std::vector<TableNode*>& in_nodes,
                                   const std::function<bool(TableNode*)>& node_func )
{
    std::vector<TableNode*> nodes;
    
    for ( auto node : in_nodes ) {
        std::vector<TableNode*> border_nodes;
        getAdjacentNodes( node, border_nodes, true );
        for ( auto nnode : border_nodes ) {
            if ( nnode && node_func(nnode) ) {
                nodes.push_back(nnode);
            }
        }
    }
    {
        std::sort(nodes.begin(), nodes.end());
        auto iter = std::unique(nodes.begin(), nodes.end());
        nodes.resize(std::distance(nodes.begin(),iter));
    }
    {
        std::sort(in_nodes.begin(), in_nodes.end());
        out_nodes.resize( out_nodes.size() + nodes.size() + in_nodes.size() );
        
        auto iter = std::set_difference( nodes.begin(), nodes.end(),
                                         in_nodes.begin(), in_nodes.end(),
                                         out_nodes.begin() );
        out_nodes.resize(std::distance(out_nodes.begin(),iter));
    }
}

bool TableGraph::canPaintBlock(TableBlock *block)
{
    return ( canSwapBlock(block) &&
            block->getColor() != getFruitColor() &&
            !block->isObjective() && block->getType() != OMNI && !block->isPetrified());
}

bool TableGraph::canHammerBlock(TableBlock *block)
{
    if ( block && !block->isInvinsible() )
        return true;
    return false;
}


void TableGraph::setMatchState                         ( TableMatch* match, FallState state )
{
    for (auto block : match->blocks) {
        if (match->match_type == TableMatch::MATCH_TWO_OMNI)
            block->setInvinsible(false);
        if (state == DESTROYING) block->destroy(match->id);
        else block->setFallState(state);
    }
}

void TableGraph::setBlockState                         ( TableBlock* block, FallState state )
{
    block->setFallState(state);
}

void TableGraph::nodeStateChanged                  ( TableNode* node, TableNode::NodeState state, bool set, uint8_t inc )
{
    char txt [32];
    Vec2 pos = node->getTablePosition();
    switch (state) {
        case TableNode::FROZEN:
            if (set)
                sprintf(txt, "FROZEN");
            else
                sprintf(txt, "UNFROZEN");
            break;
        case TableNode::SHAKEN:
            if (set)
                sprintf(txt, "SHAKEN");
            else
                sprintf(txt, "UNSHAKEN");
            break;
        default:
            break;
    }
    char address[64];
    sprintf(address, "Node:%d:%d", int(pos.x),int(pos.y));
    char fcinc [4];
    switch (inc) {
        case 1: sprintf(fcinc, "++");break;
        case 2: sprintf(fcinc, "--");break;
        default: sprintf(fcinc, " ");break;
    }
    GRAPHLOG("Set Node State %p at: x: %d y: %d to %s: %d %s", node, (int)pos.x, (int)pos.y, txt, node->_freeze_count, fcinc);
    char new_txt [32];
    sprintf(new_txt, "%s %s", txt, fcinc);
    auto iter = _graph_debug.find(address);
    if (iter == _graph_debug.end()) {
        iter = _graph_debug.insert(iter,std::pair<std::string,std::vector<std::string>>(address,std::vector<std::string>()));
    }
    iter->second.push_back(new_txt);
}


void TableGraph::blockFallStateChanged                  ( TableBlock* block, FallState state, uint8_t inc )
{
    TableNode* node = block->getNode();
    Vec2 pos(0.0f,0.0f);
    if (node) pos = node->getTablePosition();
    char txt [32];
    switch (state) {
        case STATIC:
            sprintf(txt, "STATIC");
            break;
        case FALLEN:
            sprintf(txt, "FALLEN");
            break;
        case FALLING:
            sprintf(txt, "FALLING");
            break;
        case STOPPED:
            sprintf(txt, "STOPPED");
            break;
        case STOPPING:
            sprintf(txt, "STOPPING");
            break;
        case RESTING:
            sprintf(txt, "RESTING");
            break;
        case DESTROYING:
            sprintf(txt, "DESTROYING");
            break;
        case DESTROYED:
            sprintf(txt, "DESTROYED");
            break;
        default:
            break;
    }
    char address[64];
        sprintf(address, "%p", block);
    char fcinc [4];
    switch (inc) {
        case 1: sprintf(fcinc, "++");break;
        case 2: sprintf(fcinc, "--");break;
        default: sprintf(fcinc, " ");break;
    }
    GRAPHLOG("Set Block State %p at: x: %.0f y: %.0f to %s %s", block, pos.x, pos.y, txt, fcinc);
    sprintf(txt, "%s %s", txt, fcinc);
    auto iter = _graph_debug.find(address);
    if (iter == _graph_debug.end()) {
        iter = _graph_debug.insert(iter,std::pair<std::string,std::vector<std::string>>(address,std::vector<std::string>()));
    }
    iter->second.push_back(txt);
}



void TableGraph::setMatchType                          ( TableMatch* match, BlockType type )
{
    for (auto block : match->blocks) {
        block->setBlockType(type);
    }
}


void TableGraph::dispatchMatches(TableMatch* match)
{
    GRAPHLOG("match added");
    increaseFallCount();
    _delegate->blocksMatch(match);
}

void TableGraph::setNodeState ( TableNode* node, TableNode::NodeState state )
{
    node->setState(state);
}

void TableGraph::getBorderNodes(TableNode *node, std::vector<TableNode *> &adj_nodes, bool active)
{
    auto func = [active] (TableNode* node)
                 { if (node && ((active && node->isActive()) || !active)) return true; return false; };
    for (uint8_t i = 0; i < 8; i++) {
        TableNode::TableDir dir = TableNode::TableDir(i);
        TableNode* adj = node->getAdjacentNode(dir,func);
        if (adj) adj_nodes.push_back(adj);
    }
}

void TableGraph::getAdjacentNodes(TableNode *node, std::vector<TableNode *> &adj_nodes, bool active)
{
    auto func = [active] (TableNode* node)
    { if (node && ((active && node->isActive()) || !active)) return true; return false; };
    for (uint8_t i = 0; i <= 6; i+=2) {
        TableNode::TableDir dir = TableNode::TableDir(i);
        TableNode* adj = node->getAdjacentNode(dir, func);
        if (adj) adj_nodes.push_back(adj);
    }
}

TableMatch* TableGraph::createMatch()
{
    _match_counter++;
    TableMatch* match = new TableMatch();
    match->id = _match_counter;
    return match;
}

void TableGraph::destroyMatch ( TableMatch* match )
{
    //TableBlock* source = match->blocks.front();
    //TableBlock* destination = *(match->blocks.begin()+1);
    
    std::vector<TableBlock*> under_blocks;
    std::vector<TableNode*> block_nodes;
    increaseFallCount();
    
    for ( auto i : match->nodes ) {
        if ( i->getOverBlock() && i->getOverBlock()->willShakeBlock() ) {
            block_nodes.push_back(i);
        }
        if ( i->getUnderBlock() && i->getOverBlock()->willDestroyUnderBlock() ) {
            under_blocks.push_back(i->getUnderBlock());
        }
    }
    
    if (match->willDestroyUnderBlocks()) {
        destructBlocks(&under_blocks);
    }
    
    // we need to remove match two blocks silently
    if ( match->isMatchTwo() ) {
        for (auto i : match->blocks) {
            removeBlock(i,match->id,true);
        }
    } else {
        removeMatch(match);
    }
    

    _delegate->matchDestroyed(match);
    
    if (!match->isMatchTwo()) {
        shakeAdjacentNodes  (block_nodes, match->id);
    } else if (match->match_type == TableMatch::MATCH_TWO_MIRACLE) {
        std::vector<TableNode*> temp_nodes;
        temp_nodes.push_back(*(match->nodes.begin()+1));
        shakeAdjacentNodes  (temp_nodes, match->id);
    }
    evaluateBorderNodes (block_nodes);
    clearMatch(match);
    decreaseFallCount();
    //checkGraph();

}

void TableGraph::shakeAdjacentNodes( std::vector<TableNode *>& block_nodes, uint32_t idx )
{
    std::unordered_set<TableNode*> adjacent_nodes;
    
    for (auto i : block_nodes) {
        std::vector<TableNode*> border_nodes;
        getAdjacentNodes(i, border_nodes);
        adjacent_nodes.insert(border_nodes.begin(), border_nodes.end());
    }
    for (auto i : block_nodes) {
        auto node = adjacent_nodes.find(i);
        if (node != adjacent_nodes.end()) {
            adjacent_nodes.erase(node);
        }
    }
    for (auto i : adjacent_nodes) {
        i->shake(idx);
    }
}

void TableGraph::evaluateBorderNodes ( std::vector<TableNode*>& block_nodes )
{
    std::unordered_set<TableNode*> adjacent_nodes;
    
    for (auto i : block_nodes) {
        std::vector<TableNode*> border_nodes;
        getBorderNodes(i, border_nodes,false);
        adjacent_nodes.insert(border_nodes.begin(), border_nodes.end());
    }
    for (auto i = adjacent_nodes.begin(); i != adjacent_nodes.end(); i++)
    {
        (*i)->evaluateNode();
    }
}

void TableGraph::resetBlock ( TableBlock* block )
{
    _delegate->blockReset(block);
}


void TableGraph::destructBlocks ( std::vector<TableBlock*>* block_nodes )
{
    for (auto i = block_nodes->begin(); i != block_nodes->end(); i++) {
        destructBlock(*i, (*i)->getNode( ), false, 0);
    }
}

void TableGraph::destructBlock( TableBlock* block, TableNode* node, bool shake, uint32_t idx )
{
    bool block_shake = false;
    increaseFallCount();
    TableBlock* under_block = node->getUnderBlock();
    bool destroy_block = true;
    if ( block && block != under_block ) {
        if (block->destroy(idx)) {
            block_shake = block->willShakeBlock();
            destroy_block = block->willDestroyUnderBlock();
            _delegate->blockDestroyed(block);
            destroyBlock(block,idx);
        } else {
            under_block = nullptr;
            _delegate->blockDestroyed(block);
        }
    }
    if ( under_block && destroy_block ) {
        if (under_block->destroy(idx)) {
            _delegate->blockDestroyed(under_block);
            destroyBlock(under_block,idx);
        } else {
            _delegate->blockDestroyed(under_block);
        }
    }
    std::vector<TableNode*> nodes;
    nodes.push_back(node);
    if (shake && block_shake ) {
        shakeAdjacentNodes(nodes, idx);
    }
    evaluateBorderNodes(nodes);
    decreaseFallCount();
}

void TableGraph::removeMatch ( TableMatch* match )
{
    for (auto i : match->blocks) {
        removeBlock(i, match->id);
    }
}

void TableGraph::removeBlock                            ( TableBlock* block, uint32_t idx, bool silent )
{
    if (block->isInvinsible()) {
        return;
    }
    TableNode* node = block->getNode();
        
    bool overblock = true;
    if (node->getOverBlock() == block) {
        node->setOverBlock(nullptr);
    } else if (node->getUnderBlock() == block){
        node->setUnderBlock(nullptr);
        overblock = false;
    } else {
        CCASSERT(block->getType() == JELLY, "block - node mismatch");
    }
        
    if (!silent) _delegate->blockRemoved(block, idx);
    
    CCASSERT(silent || block->getFallState() == DESTROYING, "bad state");

    if (block->getDelegate()) { _delegate->blockErased(block); }
    
}

void TableGraph::clearBlock                            ( TableBlock* block )
{
    if (block->isInvinsible()) {
        return;
    }

    block->setFallState(DESTROYED);
    
    auto iter = std::find(_over_blocks.begin(), _over_blocks.end(), block);
    if (iter != _over_blocks.end()) {
        _over_blocks.erase(iter);
    } else {
        iter = std::find(_under_blocks.begin(), _under_blocks.end(), block);
        if (iter != _under_blocks.end()) {
            _under_blocks.erase(iter);
        } else {
            CCASSERT(false, "block not found");
        }
    }
    
    delete block;
}


void TableGraph::clearMatch(TableMatch *match)
{
    for (auto i : match->blocks) {
        clearBlock(i);
    }
    delete match;
    decreaseFallCount();
    GRAPHLOG("match removed");
}

/*std::vector<TableNode *>* TableGraph::createAction(std::vector<TableNode *> &block_nodes)
{
    std::vector<TableNode*>* action = new std::vector<TableNode*>(block_nodes);
    _actions.push_back(action);
    return action;
}*/


void TableGraph::destroyBlock(TableBlock *block, uint32_t idx )
{
    if ( !block ) return;
    TableNode* node = block->getNode();
    if (block->isPetrified()) {
        block->petrify(false);
        block->immunized(idx);
        blockAdded(block);
        return;
    }
    if (node->getOverBlock() == block) {
        removeBlock(block, idx);
        evaluateBorderNodes(node);
    }
    else removeBlock(block, idx);
    clearBlock(block);
}


void TableGraph::calcHintPriority ( TableHint* hint )
{
    for (auto i : hint->match->blocks) {
        if (i->isObjective())
            hint->priority++;
    }
    for (auto i : hint->match->nodes) {
        if (i->checkState(TableNode::NodeState::MISSLE))
            hint->priority++;
        if (i->getUnderBlock())
            hint->priority++;
    }
}

void TableGraph::evaluateNodes()
{
    for (auto& i : _nodes) {
        i->evaluateNode();
    }
}

void TableGraph::evaluateBorderNodes(TableNode *node)
{
    if (node->_north)       node->_north->evaluateNode      ( );
    if (node->_north_east)  node->_north_east->evaluateNode ( );
    if (node->_east)        node->_east->evaluateNode       ( );
    if (node->_south_east)  node->_south_east->evaluateNode ( );
    if (node->_south)       node->_south->evaluateNode      ( );
    if (node->_south_west)  node->_south_west->evaluateNode ( );
    if (node->_west)        node->_west->evaluateNode       ( );
    if (node->_north_west)  node->_north_west->evaluateNode ( );
    
    node->evaluateNode              ( );
}

void TableGraph::increaseFallCount( )
{
    if (_fall_count == 0 ) {
        _table_state = ACTIVE;
        _delegate->graphActive();
    }
    _fall_count++;
    
    GRAPHLOG("Fall count increase = %d", _fall_count);
}

void TableGraph::decreaseFallCount( )
{ 
    if (_fall_count > 0) {
        _fall_count--;
        GRAPHLOG("Fall count decrease = %d", _fall_count);
    }
    if (_fall_count == 0) {
        checkGraph();
    }
}

void TableGraph::printGraph()
{
    for (auto i : _graph_debug) {
        GRAPHLOG( "%s", i.first.c_str() );
        for ( auto j : i.second ) {
            GRAPHLOG( "%s", j.c_str() );
        }
        GRAPHLOG( "----------------" );
    }
}

void TableGraph::checkGraph()
{
    if (_fall_count == 0 &&
        _table_state != READY) {
        _table_state = READY;
        
#if defined (COCOS2D_DEBUG) && (COCOS2D_DEBUG > 0) && (GRAPH_DEBUG) && (GRAPH_DEBUG > 0)
        printGraph();
        
        for (auto i : _nodes) {
            CCASSERT(!i->checkState(TableNode::FROZEN) && !i->checkState(TableNode::SHAKEN), "Nodes active");
        }
        for (auto i : _over_blocks) {
            CCASSERT(i->getFallState() == RESTING || i->getFallState() == STATIC || i->getNode(), "Blocks Active");
        }
        for (auto i : _under_blocks) {
            CCASSERT(i->getFallState() == RESTING || i->getFallState() == STATIC || i->getNode(), "Blcoks Active");
        }
#endif
        _graph_debug.clear();
        _delegate->graphReady();
        GRAPHLOG("Graph ready");
    }
}

void TableGraph::swapBlocks( TableNode *t1, TableNode *t2 )
{
    TableBlock* _dest_block = t1->getOverBlock();
    TableBlock* _source_block = t2->getOverBlock();
    t2->setOverBlock(_dest_block);
    t1->setOverBlock(_source_block);
}
void TableGraph::eraseBlock(TableBlock *block)
{
    if (block) {
        removeBlock(block,0,true);
        clearBlock(block);
    }
}

void TableGraph::shakeBlock(TableBlock *block, TableNode *node, uint32_t idx) {
    if ( !block || !block->willBlockShake( ) ) return;
    _delegate->blockShake( block, node, idx );
}


void TableGraph::setFruitColor( BlockColor color )
{
    _fruit_color = color;
    //spawnFruit();
}

void TableGraph::checkForMatchTwoType(TableBlock *destination_block, TableBlock *source_block,
                                      TableMatch::TableMatchType& match_type, BlockType& type)
{
    
    if (source_block->getType() == OMNI && (destination_block->getType() == OMNI || destination_block->getType() == MANA)) {
        match_type = TableMatch::MATCH_TWO_OMNI;
    } else if ( source_block->getType() == MANA &&
                destination_block->getColor() < num_block_colors &&
                !destination_block->isObjective() &&
                static_cast<TableBlockMana*>(source_block)->getSourceColor() != destination_block->getColor() ) {
        match_type = TableMatch::MATCH_TWO_MANA;
        type = destination_block->getType();
    } else if ( source_block->getType() == OMNI && !destination_block->isObjective() ) {
        match_type = TableMatch::MATCH_OMNI;
        type = destination_block->getType();
    } else if ( source_block->getType() == MIRACLE && destination_block->getColor() < num_block_colors &&
               !destination_block->isObjective() ) {
        match_type = TableMatch::MATCH_TWO_MIRACLE;
        type = destination_block->getType();
    }
    // omni is the only match two that excepts normal blocks so early out to avoid a lot of extra checks
    else if (source_block->getType() == NORMAL || destination_block->getType() == NORMAL ) {
        return;
    } else if ((source_block->getType() == VERTICAL || source_block->getType() == HORIZONTAL) &&
             (destination_block->getType() == VERTICAL || destination_block->getType() == HORIZONTAL)) {
        match_type = TableMatch::MATCH_TWO_X;
    } else if (source_block->getType() == CROSS && destination_block->getType() == BLAST) {
        match_type = TableMatch::MATCH_TWO_X_BIG;
    } else if (source_block->getType() == VERTICAL && destination_block->getType() == BLAST) {
        match_type = TableMatch::MATCH_TWO_BLAST_VERTICAL;
    } else if (source_block->getType() == HORIZONTAL && destination_block->getType() == BLAST) {
        match_type = TableMatch::MATCH_TWO_BLAST_HORIZONTAL;
    } else if (source_block->getType() == MISSLE && destination_block->getType() == BLAST) {
        match_type = TableMatch::MATCH_TWO_MISSLE_BLAST;
    } else if (source_block->getType() == MISSLE && destination_block->getType() == HORIZONTAL) {
        match_type = TableMatch::MATCH_TWO_MISSLE_HORIZONTAL;
    } else if (source_block->getType() == MISSLE && destination_block->getType() == VERTICAL) {
        match_type = TableMatch::MATCH_TWO_MISSLE_VERTICAL;
    } else if (source_block->getType() == MISSLE && destination_block->getType() == CROSS) {
        match_type = TableMatch::MATCH_TWO_MISSLE_CROSS;
    } else if (source_block->getType() == MISSLE && destination_block->getType() == MISSLE) {
        match_type = TableMatch::MATCH_TWO_MISSLE_MISSLE;
    } else if (( source_block->getType() == HORIZONTAL || source_block->getType() == VERTICAL ||
                source_block->getType() == CROSS )
               && destination_block->getType() == CROSS) {
        match_type = TableMatch::MATCH_TWO_STAR;
    } else if (source_block->getType() == BLAST && destination_block->getType() == BLAST) {
        match_type = TableMatch::MATCH_TWO_BLAST_BIG;
    }
}

TableMatch* TableGraph::checkForMatchTwo(TableBlock *source_block, TableBlock *destination_block)
{
    if ( !source_block->willMatchBlock() || !destination_block->willMatchBlock() ) return nullptr;
    TableMatch::TableMatchType match_type = TableMatch::MATCH_NONE;
    BlockType type = NORMAL;

    checkForMatchTwoType(source_block, destination_block, match_type, type);
    if ( match_type == TableMatch::MATCH_NONE ) {
        std::swap(source_block, destination_block);
        checkForMatchTwoType(source_block, destination_block, match_type, type);
        std::swap(source_block, destination_block);
    }
    if (match_type != TableMatch::MATCH_NONE) {
        TableMatch* match = createMatch();
        match->source_color = source_block->getColor();
        match->target_color = destination_block->getColor();
        match->type = type;
        if (source_block) {
            match->nodes.push_back(source_block->getNode());
            match->blocks.push_back(source_block->getNode()->getOverBlock());
        }
        if (destination_block) {
            match->nodes.push_back(destination_block->getNode());
            match->blocks.push_back(destination_block->getNode()->getOverBlock());

        }
        match->match_type = match_type;
        return match;
    }
    return nullptr;
}

bool TableGraph::willMatchThree(TableBlock* source_block, TableBlock* destination_block)
{
    TableMatch* match_1 = nullptr;
    TableMatch* match_2 = nullptr;
    
    bool match = false;
    swapBlocks(source_block->getNode(), destination_block->getNode());
    match_1 = checkForMatchs(source_block->getNode(),true);
    match_2 = checkForMatchs(destination_block->getNode(),true);
    swapBlocks(source_block->getNode(), destination_block->getNode());

    if (match_1 || match_2) match = true;
    
    delete match_1;
    delete match_2;
    
    return match;
    
}

bool TableGraph::blockAdded(TableBlock *source)
{
    if (!source || source->getFallState() != RESTING ) return false;
    
    TableMatch* match_1 = checkForMatchs(source->getNode(),true);
    
    if (!match_1) return false;

    setMatchState( match_1, DESTROYING );
    
    dispatchMatches ( match_1 );
    
    return true;
}

bool TableGraph::blockMoved( TableBlock* source_block, TableBlock* destination_block, bool match_two )
{
    TableMatch* match_1 = nullptr;
    TableMatch* match_2 = nullptr;
    
    if (match_two) {
        match_1 = checkForMatchTwo(source_block, destination_block);
    }
    
    if (!match_1) {
        swapBlocks(source_block->getNode(), destination_block->getNode());
        match_1 = checkForMatchs(source_block->getNode(),false);
        match_2 = checkForMatchs(destination_block->getNode(),false);
    }

    if (!match_1 && !match_2) {
        swapBlocks(source_block->getNode(), destination_block->getNode());
        return false;
    }
    /*if ( match_1 ) {
        setMatchState(match_1, DESTROYING);
    }
    if ( match_2 ) {
        setMatchState(match_2, DESTROYING);
    }*/
    if ( match_1 ) {
        dispatchMatches ( match_1 );
    }
    if ( match_2 ) {
        dispatchMatches ( match_2 );
    }
    
    return true;
}

TableBlock* TableGraph::createTableBlock(BlockColor color, BlockType type)
{
    TableBlock* block = TableBlock::createTableBlock(color,type);
    if (block->isOverBlock())
        _over_blocks.insert(block);
    else _under_blocks.insert(block);
    return block;
}

TableHint* TableGraph::createHint( BlockColor color )
{
    std::vector<TableHint*> hints;
    TableHint* hint = nullptr;
    
    if (color == NONE) {
        for (auto i = _nodes.begin(); i != _nodes.end(); i++) {
            checkForDirectionHint(*i, hints);
        }
    } else {
        for (auto i = _nodes.begin(); i != _nodes.end(); i++) {
            checkForColorHint(*i, hints,color);
        }
    }
    if ( !hints.empty( ) ) {
        std::random_shuffle(hints.begin(), hints.end());
        std::sort(hints.begin(), hints.end(), TableHint::compare);
        
        hint = hints.back();
        hints.pop_back();
        for (auto i : hints ) {
            delete i;
        }
    }
    return hint;
}

bool TableGraph::requestHint( BlockColor color )
{
    TableHint* hint = createHint(color);
    if (hint)
        _delegate->blocksHint(hint);
    return hint;
}

void TableGraph::shuffleBlock(TableBlock *block)
{
    // clear out all blocks and roll new one's then call front-end to make transition
    if (block) {
        //block->clearDelegate();
        block->setBlockColor(BlockColor(rand() % TableBlock::num_block_colors));
    }
}


void TableGraph::spawnBlock ( TableNode* node )
{
    
    replaceBlock(node, node->getSpawner()->getNextColor(), node->getSpawner()->getNextType());
}

uint16_t TableGraph::hasOverBlock(BlockColor color, BlockType type, FallState state, bool prisoners )
{
    uint16_t count = 0;
    for (auto i : _over_blocks) {
        if (i->getType() == type &&
            i->getColor() == color &&
            i->getFallState() == state &&
            (prisoners ||
             (!prisoners && i->getNode()->getOverBlock() == i)))
            count++;
    }
    return count;
}

bool TableGraph::hasSpawnBlock (BlockColor color, BlockType type)
{
    for ( auto i : _nodes ) {
        if (i->willSpawnBlock(color, type)) return true;
    }
    return false;
}

void TableGraph::setDelegate                            ( TableGraphDelegate* delegate )
{
    _delegate = delegate;
}


TableBlock* TableGraph::insertBlock ( TableNode* node, BlockColor color, BlockType type )
{
    TableBlock* new_block = createTableBlock(color, type);
    insertBlock(node, new_block );
    return new_block;
}

TableBlock* TableGraph::replaceBlock ( TableNode* node, BlockColor color, BlockType type )
{
    if (node->getOverBlock()) {
        if ( color == node->getOverBlock()->getColor() && type == node->getOverBlock()->getType() )
            return node->getOverBlock();
    }
    TableBlock* new_block = createTableBlock(color, type);
    replaceBlock(node, new_block );
    return new_block;
}


void TableGraph::insertBlock ( TableNode* node, TableBlock* block )
{
    TableNode* new_target = node;
    
    if ( node->getOverBlock() ) {
        auto func = [] (TableNode* node) {
            if ( node && node->isActive() && !node->getOverBlock() && node->isContainer() ) return true;
            return false;
        };
        new_target = node->getAdjacentNode(node->getDirection(), func);
        
        if (!new_target) {
            TableNode* new_targets [2] = { nullptr, nullptr };
            switch (node->getDirection()) {
                case TableNode::NORTH:
                case TableNode::SOUTH: {
                    new_targets [0] = node->getAdjacentNode(TableNode::EAST, func);
                    new_targets [1] = node->getAdjacentNode(TableNode::WEST, func);
                }
                    break;
                case TableNode::EAST:
                case TableNode::WEST: {
                    new_targets [0] = node->getAdjacentNode(TableNode::NORTH, func);
                    new_targets [1] = node->getAdjacentNode(TableNode::SOUTH, func);
                }
                    break;
                default:
                    break;
            }
            
            if ( new_targets[0] && new_targets[1] ) {
                new_target = new_targets[rand() % 2];
            } else if (new_targets[0]) {
                new_target = new_targets[0];
            } else if (new_targets[1]) {
                new_target = new_targets[1];
            } else {
                new_target = node->getAdjacentNode(TableNode::getInverseDirection(node->getDirection()), func);
            }
        }
    }
    
    if ( new_target == nullptr ) {
        GRAPHLOG("Could not insert block");
        eraseBlock(block);
        return;
    }
    
    if (node != new_target) {
        blockDelegateToNode (block, node);
        dropBlock(block, node, new_target);
    } else {
        replaceBlock(node, block);
    }
}

void TableGraph::blockDelegateToNode (TableBlock* block, TableNode* node)
{
    TableBlockDelegate* block_del = block->getDelegate();
    if (!block_del) {
        block_del = _delegate->blockCreated(block, node);
    } else {
        _delegate->blockInserted(block,node,!block->isOverBlock());
    }
}


void TableGraph::emplaceBlock(TableNode *node, TableBlock *block)
{
    if (node->getOverBlock()) {
        eraseBlock(node->getOverBlock());
    }
    blockDelegateToNode (block, node);
    
    node->setOverBlock(block);
    
    spawnBlockChanged(block, node);
}

void TableGraph::replaceBlock(TableNode* node, TableBlock* block )
{
    emplaceBlock(node, block);
    if (_fall_count > 0 && node->isActive()) {
        _delegate->blockStopped(block);
    }
}

TableBlock* TableGraph::spawnBlock(TableNode *node, TableBlock *block)
{
    TableBlock* new_block = nullptr;
    if ( node->isSpawner() ) {
        _delegate->spawningBlock(node);
        TableSpawn* spawner = node->getSpawner();
        if ( spawner->pendingSpawn() ) {
            new_block = createTableBlock(spawner->getNextColor(), spawner->getNextType());
            spawner->setPending(false);
            if (block) {
                new_block->setFallCount(block->getFallCount());
                eraseBlock(block);
            }
        }
    }
    return new_block;
}

void TableGraph::spawnBlockChanged ( TableBlock* block, TableNode* node )
{
    if ( node->isSpawner() ) {
        if (node->getSpawner()) {
            node->getSpawner()->setNextType(block->getType());
            node->getSpawner()->setNextColor(block->getColor());
        }
        _delegate->spawnColorChanged(node);
    }
}

void TableGraph::dropBlock ( TableBlock* block, TableNode* source, TableNode* target )
{
    _delegate->blockMoveToTarget(block->getDelegate(), source->getDelegate(), target->getDelegate());
    TableNode* prev_node = block->getNode();
    if (prev_node) {
        prev_node->setOverBlock(nullptr);
    }
    target->setOverBlock(block);
    target->getOverBlock()->setFallState(FALLING);
    
    spawnBlockChanged(block, target);
    
    if (prev_node) {
        evaluateBorderNodes(prev_node);
    }
}

void TableGraph::stopBlock( TableNode *node )
{
    if (node->getOverBlock()->getFallState() == FALLEN) {
        node->getOverBlock()->setFallState(STOPPING);
        _delegate->blockStopped(node->getOverBlock());
    } else if (node->getOverBlock()->getFallState() == STOPPED) {
        TableMatch* match = checkForMatchs(node, true);
        if (match) {
            //it's possible to get a match that doesn't include the orginal node. If this happens we need to make sure the block goes into rest state.
            bool match_node = false;
            for (auto i : match->nodes) {
                if (i == node) { match_node = true; break; }
            }
            setMatchState(match, DESTROYING);
            dispatchMatches(match);
            if (!match_node && node->getOverBlock()) {
                stopBlock( node );
                return;
            }
        } else {
            node->getOverBlock()->setFallState(RESTING);
        }
    }
}

bool TableGraph::spawnBlockAtRandomNode(BlockColor color, BlockType type)
{
    std::vector<TableNode*> spawners;
    if (TableBlock::isObjective(type)) {
        getSpawners(spawners,3);
    } else {
        getSpawners(spawners,2);
    }
    if (spawners.size() == 0) return false;
    int pos = rand() % int (spawners.size());
    TableNode* spawner = spawners.at(pos);
    spawner->setNextSpawnBlock(color, type, false);
    //replaceBlock(spawner, color, type);
    spawnBlock(spawner);
    return true;
}

TableNode* TableGraph::findTrackNode ( TableNode* node )
{
    if (node->checkState(TableNode::CAM_TRACK) && node->_cam_idx == _cam_group) return node;
    
    TableNode* best_track = nullptr;
    float best_dist = MAXFLOAT;
    for (uint16_t y = MAX(0, node->y - 5); y < MIN(node->y + 5, _graph_dim.y); y++) {
        for (uint16_t x = MAX(0, node->x - 5); x < MIN(node->x + 5, _graph_dim.x); x++) {
            TableNode* obj_bounds = getNodeByPosition(Vec2(x,y));
            if (!obj_bounds->checkState(TableNode::CAM_TRACK) ||
                obj_bounds->_cam_idx != _cam_group) {
                continue;
            }
            float dist = node->getTablePosition().distance(obj_bounds->getTablePosition());
            if ( dist < best_dist ) {
                best_track = obj_bounds;
                best_dist = dist;
            }
            /*switch (node->getDirection()) {
                case TableNode::NORTH:
                case TableNode::SOUTH:
                    if (!best_track || (best_track && abs(best_track->y - node->y) > abs(obj_bounds->y - node->y)))
                        best_track = obj_bounds;
                    break;
                case TableNode::WEST:
                case TableNode::EAST:
                    if (!best_track || (best_track && abs(best_track->x - node->x) > abs(obj_bounds->x - node->x)))
                        best_track = obj_bounds;
                    break;
                default:
                    CCASSERT(false, "direction not supported");
                    break;
            }*/
        }
    }
    return best_track;
}

TableNode* TableGraph::calcCurrentJumpNode()
{
    for ( auto node: _nodes ) {
        if ( node->_cam_idx == _cam_group ) {
            return node;
        }
    }
    return nullptr;
}


TableNode* TableGraph::calcCurrentTrackNode()
{
    // little data structure to help with sorting. It contains the objective block's node,
    // the best cam track node for that node and the distance the cam track node to the track root node.
    struct objective { TableNode* obj_node = nullptr; TableNode* track_node = nullptr; uint16_t dist = 0;
        static bool compare (const objective& lhs, const objective&rhs)
        {
            return (lhs.dist > rhs.dist);
        }
    };
    
    // find all nodes that have objective blocks in the current camera group.
    std::vector<objective> objectives;
    for ( auto node: _nodes ) {
        if ( node->getOverBlock() &&
            node->getOverBlock()->isObjective() &&
            node->_cam_idx == _cam_group ) {
            objective obj;
            obj.obj_node = node;
            objectives.push_back(obj);
        }
    }
    
    // if we can't find any objectives return the node at start of camera track
    if (objectives.empty()) return getTrackStartNode();
    
    // now lets find the best camera track position for each objective.
    for ( auto& node : objectives ) {
        node.track_node = findTrackNode(node.obj_node);
        CCASSERT(node.track_node, "no camera node found for objective");
        node.dist = findTrackEnd(node.track_node, nullptr);
    }
    
    // lets sort the objectives by the distance from the cam track node to the root cam node.
    
    std::sort(objectives.begin(), objectives.end(), objective::compare);
    
    // so the objective at the start of the list is the one we are concerned with, but lets see if we can safely fit
    // more onto the screen.
    
    if ( objectives.front().dist > 0 ) {
        return advanceTradeNode(objectives.front().track_node,2);
    } else {
        return objectives.front().track_node;
    }
    
}

TableNode* TableGraph::getTrackStartNode()
{
    return _cam_nodes[_cam_group];
}

TableNode* TableGraph::getTrackEndNode()
{
    TableNode* cam_node = nullptr;
    
    findTrackEnd(_cam_nodes[_cam_group], &cam_node,true);
    
    return cam_node;
}

void TableGraph::replaceBlockType( BlockType old_type, BlockType new_type, ActiveState active )
{
    for (auto i : _nodes) {
        if ( i->getOverBlock() &&
            ((active == ACTIVE_ONLY && i->isActive()) ||
             (active == INACTIVE_ONLY && !i->isActive())) &&
            !i->getOverBlock()->checkState(IMMUTABLE) && i->getOverBlock()->getType() == old_type) {
            i->getOverBlock()->setBlockType(new_type);
            if (i->getOverBlock()->getDelegate()) {
                _delegate->blockErased(i->getOverBlock());
                i->getOverBlock()->setDelegate(_delegate->blockCreated(i->getOverBlock(), i, false));
            }
        }
    }
}

void TableGraph::clearMatches( ActiveState active )
{
    std::function<bool(TableNode*)> node_func = nullptr;
    std::function<bool(TableBlock*)> block_func = nullptr;
    getShuffleFilter ( node_func, block_func, active );
    
    std::set<BlockColor> all_colors;
    for (uint8_t color = 0; color < TableBlock::num_block_colors; color++) {
        all_colors.insert(BlockColor(color));
    }
    bool check_again = true;
    while (check_again) {
        check_again = false;
        for (auto i : _nodes) {
            if (!node_func(i) || !block_func(i->getOverBlock()))
                continue;
            TableMatch* match = checkForMatchs(i, false, NONE, false);
            if (match)
            {
                delete match;
                std::set<BlockColor> colors;
                checkAdjColors(i, colors);
                colors.insert(i->getOverBlock()->getColor());
                if (colors.size() == 0) continue;
                //std::sort(colors.begin(), colors.end());
                std::vector<BlockColor> diffcolors;
                std::set_symmetric_difference(colors.begin(), colors.end(),
                                              all_colors.begin(), all_colors.end(),
                                              std::inserter(diffcolors, diffcolors.end()));
                if (diffcolors.empty()) {
                    int color = rand() % (TableBlock::num_block_colors - 1);
                    if (color >= i->getOverBlock()->getColor()) color+=1;
                    i->getOverBlock()->setBlockColor(BlockColor(color));
                    check_again = true;
                    continue;
                }
                
                i->getOverBlock()->setBlockColor(*diffcolors.begin());
                if ( i->getOverBlock()->getDelegate() && active != ACTIVE_ONLY ) {
                    _delegate->blockErased(i->getOverBlock());
                    i->getOverBlock()->setDelegate(_delegate->blockCreated(i->getOverBlock(), i, false));
                }
                //CCASSERT(i->getOverBlock()->getDelegate() == nullptr, "block has a delegate. clean this up BEFORE running this function");
            }
        }
    }
    
    
}

void TableGraph::checkForColorHint ( TableNode* node, std::vector<TableHint*>& hints, BlockColor color )
{
    if ( !node->getOverBlock() ||
        !isNormalBlock(node->getOverBlock()) || !node->isActive() ) return;
    
    TableMatch* match = checkForMatchs(node, false, color);
    
    if (match && match->isHint()) {
        TableHint* hint = new TableHint();
        hint->match = match;
        hint->source = node;
        calcHintPriority(hint);
        hints.push_back(hint);
    } else {
        delete match;
    }
    
}

void TableGraph::checkForDirectionHint ( TableNode* node, std::vector<TableHint*>& hints )
{
    if ( !node->getOverBlock() || !node->getOverBlock()->willMoveBlock() || !node->isActive() ) return;
    TableMatch* match = nullptr;
    TableNode::TableDir dir;
    TableNode* target = nullptr;
    for (uint8_t i = 0; i < 4 ; i++) {
        switch (i) {
            case 0:
                target = node->_north;
                dir = TableNode::NORTH;
                break;
            case 1:
                target = node->_east;
                dir = TableNode::EAST;
                break;
            case 2:
                target = node->_south;
                dir = TableNode::SOUTH;
                break;
            case 3:
                target = node->_west;
                dir = TableNode::WEST;
            default:
                break;
        }
        if ( !target || !target->isActive() || !target->getOverBlock() || !target->getOverBlock()->willMoveBlock() ) continue;
        
        match = checkForMatchTwo(node->getOverBlock(), target->getOverBlock());
        if (!match) {
            swapBlocks(node, target);
            match = checkForMatchs(target, false);
            swapBlocks(node, target);
        }
        
        if (match) {
            TableHint* hint = new TableHint();
            hint->match = match;
            hint->source = node;
            hint->dir = dir;
            calcHintPriority(hint);
            hints.push_back(hint);
        } else {
            delete match;
        }
    }
}

void TableGraph::lTrimAxialMatchs ( AxialMatch& out_match )
{
    TableNode* target = nullptr;
    if (out_match.north_matches.size() == 1) { target = out_match.north_matches.at(0); out_match.north_matches.clear(); }
    if (out_match.south_matches.size() == 1) { target = out_match.south_matches.at(0); out_match.south_matches.clear(); }
    if (target) {
        auto iter = std::find(out_match.vertical_matches.begin(), out_match.vertical_matches.end(), target);
        if (iter != out_match.vertical_matches.end()) {
            out_match.vertical_matches.erase(iter);
        }
    }
    if (out_match.east_matches.size() == 1) { target = out_match.east_matches.at(0); out_match.east_matches.clear(); }
    if (out_match.west_matches.size() == 1) { target = out_match.west_matches.at(0); out_match.west_matches.clear(); }
    if (target) {
        auto iter = std::find(out_match.horizontal_matches.begin(), out_match.horizontal_matches.end(), target);
        if (iter != out_match.horizontal_matches.end()) {
            out_match.horizontal_matches.erase(iter);
        }
    }
}

TableNode* TableGraph::tTrimAxialMatchs ( AxialMatch& out_match )
{
    TableNode* target = nullptr;
    if (out_match.north_matches.size() == 1 && out_match.south_matches.size() == 1) {
        if (out_match.east_matches.size() == 2 && out_match.west_matches.size() == 1) {
            target = out_match.west_matches.at(0);
            out_match.west_matches.clear();
        }
        else if (out_match.west_matches.size() == 2 && out_match.east_matches.size() == 1) {
            target = out_match.east_matches.at(0);
            out_match.east_matches.clear();
        }
        if (target) {
            out_match.horizontal_matches.erase(std::find(out_match.horizontal_matches.begin(), out_match.horizontal_matches.end(), target));
        }
    } else if (out_match.east_matches.size() == 1 && out_match.west_matches.size() == 1) {
        if (out_match.north_matches.size() == 2 && out_match.south_matches.size() == 1) {
            target = out_match.south_matches.at(0);
            out_match.south_matches.clear();
        }
        else if (out_match.south_matches.size() == 2 && out_match.north_matches.size() == 1) {
            target = out_match.north_matches.at(0);
            out_match.north_matches.clear();
        }
        if (target) {
            out_match.vertical_matches.erase(std::find(out_match.vertical_matches.begin(), out_match.vertical_matches.end(), target));
        }
    }
    return target;
    
}

void TableGraph::checkForAxialMatchs( TableNode *node, BlockColor color, AxialMatch &out_match, bool falling, bool active )
{
    out_match.vertical_matches.clear();
    out_match.horizontal_matches.clear();
    out_match.north_matches.clear();
    out_match.south_matches.clear();
    out_match.east_matches.clear();
    out_match.west_matches.clear();
    
    checkAdjDir(node, color, TableNode::NORTH, out_match.north_matches, falling, active);
    checkAdjDir(node, color, TableNode::SOUTH, out_match.south_matches, falling, active);
    checkAdjDir(node, color, TableNode::EAST, out_match.east_matches, falling, active);
    checkAdjDir(node, color, TableNode::WEST, out_match.west_matches, falling, active);
    
    
    for ( uint8_t i = 0; i < MAX(out_match.north_matches.size(), out_match.south_matches.size()); i++ ) {
        if (i < out_match.north_matches.size()) out_match.vertical_matches.push_back(out_match.north_matches.at(i));
        if (i < out_match.south_matches.size()) out_match.vertical_matches.push_back(out_match.south_matches.at(i));
    }
    for ( uint8_t i = 0; i < MAX(out_match.east_matches.size(), out_match.west_matches.size()); i++ ) {
        if (i < out_match.east_matches.size()) out_match.horizontal_matches.push_back(out_match.east_matches.at(i));
        if (i < out_match.west_matches.size()) out_match.horizontal_matches.push_back(out_match.west_matches.at(i));
    }
    /*out_match.vertical_matches = out_match.north_matches;
    out_match.horizontal_matches = out_match.east_matches;
    out_match.vertical_matches.insert(out_match.vertical_matches.end(), out_match.south_matches.begin(), out_match.south_matches.end());
    out_match.horizontal_matches.insert(out_match.horizontal_matches.end(), out_match.west_matches.begin(), out_match.west_matches.end());*/
}

TableMatch* TableGraph::checkForMatchs( TableNode* node, bool falling, BlockColor color, bool active )
{
    if ( node == nullptr || !node->isContainer() || node->getOverBlock() == nullptr || (active && !node->isActive()) || !node->getOverBlock()->willMatchBlock()) return nullptr;
    
    if (color == NONE) {
        color = node->getOverBlock()->getColor();
        if (color == NONE) return nullptr;
    }    
    AxialMatch axis_match;
    
    
    // get a list of blocks that match in horizontal and vertical directions of the node. This will be the basis on which we determine our matches.
    
    TableNode* curr_node = node;
    checkForAxialMatchs(curr_node, color, axis_match, falling, active);
    
    // falling matchs may not have first block as intersection of cross pattern matches. we need to do more extensive check by iterating through each block and checking for matches.
    
    if (falling && (axis_match.horizontal_matches.size() >= 2 || axis_match.vertical_matches.size() >= 2)) {
        uint8_t dir = 0;
        if (axis_match.vertical_matches.size() < 4 && axis_match.horizontal_matches.size() < 2)
            dir = 1;
        else if (axis_match.horizontal_matches.size() < 4 && axis_match.vertical_matches.size() < 2)
            dir = 2;
        
        if (dir != 0) {
            AxialMatch alt_axis_match;
            AxialMatch best_axis_match;
            TableNode* best_node = nullptr;
            
            std::vector<TableNode*>* matchs =           nullptr;
            std::vector<TableNode*>* alt_matchs =       nullptr;
            std::vector<TableNode*>* new_matchs =       nullptr;
            std::vector<TableNode*>* new_alt_matches =  nullptr;
            std::vector<TableNode*>* best_matchs =      nullptr;
            std::vector<TableNode*>* best_alt_matches = nullptr;
            switch (dir) {
                case 1:
                    matchs =            &axis_match.vertical_matches;
                    new_matchs =        &alt_axis_match.vertical_matches;
                    best_matchs =       &best_axis_match.vertical_matches;
                    alt_matchs =        &axis_match.horizontal_matches;
                    new_alt_matches =   &alt_axis_match.horizontal_matches;
                    best_alt_matches =  &best_axis_match.horizontal_matches;
                    break;
                case 2:
                    matchs =            &axis_match.horizontal_matches;
                    new_matchs =        &alt_axis_match.horizontal_matches;
                    best_matchs =       &best_axis_match.horizontal_matches;
                    alt_matchs =        &axis_match.vertical_matches;
                    new_alt_matches =   &alt_axis_match.vertical_matches;
                    best_alt_matches =  &best_axis_match.vertical_matches;
                    break;
            }
            // check for cross pattern
            for (auto i : *matchs) {
                checkForAxialMatchs(i, color, alt_axis_match, falling, active);
                if (( new_alt_matches->size() >= 2 && new_alt_matches->size() > best_alt_matches->size() ) ||
                    ( best_axis_match.crossPattern() && new_alt_matches->size() == best_alt_matches->size() &&
                     !alt_axis_match.crossPattern()) ) {
                    // in the case of a 3 x 4 match we need to prune off one node. If that trimmed node is
                    // the same as our target node then ignore the match.
                    best_axis_match = alt_axis_match;
                    best_node = i;
                }
                // we may even find that there is better block that could spawn if we ignore the match
                /*else if ( new_alt_matches->size() >= 4 ) {
                    return nullptr;
                }*/
            }
            if ( best_node ) {
                axis_match = best_axis_match;
                node = best_node;
                tTrimAxialMatchs(axis_match);
            }
            // check for square pattern
            if (matchs->size() == 2 && alt_matchs->size() == 0) {
                for (auto i : *matchs) {
                    //if (node->getOverBlock() == i->getOverBlock()) continue;
                    checkForAxialMatchs(i, color, alt_axis_match, falling, active);
                    
                    if (new_alt_matches->size() == 1) {
                        axis_match = alt_axis_match;
                        node = i;
                        break;
                    }
                    
                }
            }
        }
    }

    // in the case we have only one block in either direction we may still have a square match.
    
    TableNode* square = nullptr;
    if (( axis_match.horizontal_matches.size() < 3 && axis_match.vertical_matches.size() < 2 ) ||
        ( axis_match.vertical_matches.size() < 3 && axis_match.horizontal_matches.size() < 2 ) ||
          axis_match.crossPattern() ) {
        auto func = [active] (TableNode* node)
        { if (node && node->isContainer() && ((active && node->isActive()) || !active)) return true; return false; };
        for (auto x : axis_match.horizontal_matches) {
            TableNode* h [2] =
            { x->getAdjacentNode(TableNode::NORTH,func),
              x->getAdjacentNode(TableNode::SOUTH,func) };
            for (auto y : axis_match.vertical_matches) {
                TableNode* v [2] =
                { y->getAdjacentNode(TableNode::EAST,func),
                  y->getAdjacentNode(TableNode::WEST,func) };
                TableBlock* h0 = nullptr;
                TableBlock* v0 = nullptr;
                TableBlock* v1 = nullptr;

                if (v[0]) v0 = v[0]->getOverBlock();
                if (v[1]) v1 = v[1]->getOverBlock();
                
                for ( uint8_t i = 0; i < 2; i++ ) {
                    if (h[i]) h0 = h[i]->getOverBlock();
                    if ( h0 && h0->getColor() == color &&
                               h0->getFallState() != DESTROYING &&
                               h0->willMatchBlock() &&
                               (h0->getFallState() != FALLING || (falling && h0->getFallState() == FALLING && _delegate->blockInNode(h0->getNode()) == h0))) {
                        if  ( h0 == v0 || h0 == v1 ) {
                            square = h0->getNode();
                            break;
                        }
                    }
                }
                if ( square ) break;
            }
            if ( square ) break;
        }
    }
    
    // in the case we have a square match and blocks are falling we may need to change the pivot to get as many blocks possible. We might even need to completely ignore the match because a better match is possible on the other corner.
    
    if ( square && falling ) {
        if ( axis_match.horizontal_matches.size() < 2 && axis_match.vertical_matches.size() < 2 ) {
            AxialMatch new_axis_match;
            curr_node = square;
            checkForAxialMatchs(curr_node, color, new_axis_match, falling, active);
            if ((new_axis_match.vertical_matches.size() > 2 || new_axis_match.horizontal_matches.size() > 2)/* ||
                (new_axis_match.vertical_matches.size() == 2 && new_axis_match.horizontal_matches.size() == 2)*/) {
                return nullptr;
            } else if ( new_axis_match.vertical_matches.size() == 2 || new_axis_match.horizontal_matches.size() == 2 ) {
                axis_match = new_axis_match;
                std::swap(node, square);
            }
        }
        
        // in the case of a square match that has a three by three cross pattern we will need to prune one of the blocks
        
        if ( axis_match.vertical_matches.size() == 2 && axis_match.horizontal_matches.size() == 2 ) {
            std::vector<TableNode*>* matchs;
            if (rand() % 2 == 0)
                matchs = &axis_match.vertical_matches;
            else
                matchs = &axis_match.horizontal_matches;
            for (auto i = matchs->begin(); i != matchs->end(); i++) {
                if ( !(*i)->isAdjacent(square) ) {
                    matchs->erase(i);
                    if (axis_match.north_matches[0] == *i) axis_match.north_matches.clear();
                    if (axis_match.south_matches[0] == *i) axis_match.south_matches.clear();
                    if (axis_match.east_matches[0] == *i) axis_match.east_matches.clear();
                    if (axis_match.west_matches[0] == *i) axis_match.west_matches.clear();
                    break;
                }
            }
        }
    }
    
    if (!square && axis_match.horizontal_matches.size() < 2 && axis_match.vertical_matches.size() < 2 ) return nullptr;
    
    TableMatch* matches = createMatch();
    
    matches->match_type = TableMatch::MATCH_THREE;
    
    matches->nodes.push_back(node);
    
    if ( axis_match.north_matches.size() >= 2 && axis_match.south_matches.size() >=2 &&
        ( axis_match.east_matches.size() >= 2 || axis_match.west_matches.size() >= 2 )) {
        GRAPHLOG("Match found at: x: %d y: %d type: %s", node->x, node->y, "match7");
        matches->match_type =  TableMatch::MATCH_SEVEN;
        //matches->nodes.push_back(axis_match.vertical_matches.at(0));
        matches->nodes.insert( matches->nodes.end(),
                              axis_match.north_matches.begin(),
                              axis_match.north_matches.begin() + 2 );
        matches->nodes.insert( matches->nodes.end(),
                              axis_match.south_matches.begin(),
                              axis_match.south_matches.begin() + 2 );
        if ( axis_match.east_matches.size() >= 2 ) {
            matches->nodes.insert( matches->nodes.end(),
                                   axis_match.east_matches.begin(),
                                   axis_match.east_matches.begin() + 2 );
        } else {
            matches->nodes.insert( matches->nodes.end(),
                                  axis_match.west_matches.begin(),
                                  axis_match.west_matches.begin() + 2 );
        }
    }
    
    else if ( axis_match.east_matches.size() >= 2 && axis_match.west_matches.size() >=2 &&
             ( axis_match.north_matches.size() >= 2 || axis_match.south_matches.size() >= 2 )) {
        GRAPHLOG("Match found at: x: %d y: %d type: %s", node->x, node->y, "match7");
        matches->match_type =  TableMatch::MATCH_SEVEN;
        //matches->nodes.push_back(axis_match.horizontal_matches.at(0));
        matches->nodes.insert( matches->nodes.end(),
                              axis_match.east_matches.begin(),
                              axis_match.east_matches.begin() + 2 );
        matches->nodes.insert( matches->nodes.end(),
                              axis_match.west_matches.begin(),
                              axis_match.west_matches.begin() + 2 );
        if ( axis_match.north_matches.size() >= 2 ) {
            matches->nodes.insert( matches->nodes.end(),
                                  axis_match.north_matches.begin(),
                                  axis_match.north_matches.begin() + 2 );
        } else {
            matches->nodes.insert( matches->nodes.end(),
                                  axis_match.south_matches.begin(),
                                  axis_match.south_matches.begin() + 2 );
        }
    }
    
    else if ( axis_match.horizontal_matches.size() >= 4 || axis_match.vertical_matches.size() >= 4 ) {
        GRAPHLOG("Match found at: x: %d y: %d type: %s", node->x, node->y, "match5 line");
        matches->match_type =  TableMatch::MATCH_FIVE_LINE;
        TableNode* iter = nullptr;
        // if the matched while falling put the pivot in the center of the match.
        if (axis_match.horizontal_matches.size() >= 4) {
            matches->nodes.insert( matches->nodes.end(),
                                   axis_match.horizontal_matches.begin(),
                                   axis_match.horizontal_matches.begin() + 4 );
            if ( falling ) {
                if ( axis_match.west_matches.size() < 2 ) iter =
                    *(axis_match.east_matches.begin() + 1 - axis_match.west_matches.size());
                else if ( axis_match.east_matches.size() < 2 ) iter =
                    *(axis_match.west_matches.begin() + 1 - axis_match.east_matches.size());
            }
        }
        else if (axis_match.vertical_matches.size() >= 4) {
            matches->nodes.insert( matches->nodes.end(),
                                  axis_match.vertical_matches.begin(),
                                  axis_match.vertical_matches.begin() + 4 );
            if ( falling ) {
                if ( axis_match.south_matches.size() < 2 ) iter =
                    *(axis_match.north_matches.begin() + 1 - axis_match.south_matches.size());
                else if ( axis_match.north_matches.size() < 2 ) iter =
                    *(axis_match.south_matches.begin() + 1 - axis_match.north_matches.size());
            }
        }
        if ( iter ) {
            auto iter2 = std::find(matches->nodes.begin() + 1, matches->nodes.end(), iter);
            if (iter2 != matches->nodes.end()) std::iter_swap(matches->nodes.begin(), iter2);
        }
    }
    
    else if (axis_match.vertical_matches.size() >= 2 && axis_match.horizontal_matches.size() >= 2 &&
             !axis_match.crossPattern()) {
        if ((axis_match.north_matches.size() == 1 && axis_match.south_matches.size() == 1) ||
            (axis_match.east_matches.size() == 1 && axis_match.west_matches.size() == 1) ) {
            GRAPHLOG("Match found at: x: %d y: %d type: %s", node->x, node->y, "match5T");
            matches->match_type =  TableMatch::MATCH_FIVE_T;
            // prune extra blocks
            if ( axis_match.vertical_matches.size() > 2 || axis_match.horizontal_matches.size() > 2 )
                tTrimAxialMatchs(axis_match);
        } else {
            GRAPHLOG("Match found at: x: %d y: %d type: %s", node->x, node->y, "match5L");
            matches->match_type =  TableMatch::MATCH_FIVE_L;
            // prune extra blocks
            if ( axis_match.vertical_matches.size() > 2 || axis_match.horizontal_matches.size() > 2 )
                lTrimAxialMatchs(axis_match);
        }
        
        matches->nodes.insert(matches->nodes.end(), axis_match.vertical_matches.begin(), axis_match.vertical_matches.begin()+2);
        matches->nodes.insert(matches->nodes.end(), axis_match.horizontal_matches.begin(), axis_match.horizontal_matches.begin()+2);
    }
    
    else if ( axis_match.vertical_matches.size() >= 3 && axis_match.horizontal_matches.size() < 2 ) {
        GRAPHLOG("Match found at: x: %d y: %d type: %s", node->x, node->y, "match4 vertical");
        matches->match_type =  TableMatch::MATCH_FOUR_VERTICAL;
        matches->nodes.insert(matches->nodes.end(), axis_match.vertical_matches.begin(), axis_match.vertical_matches.begin()+3);
    }
    
    else if ( axis_match.horizontal_matches.size() >= 3 && axis_match.vertical_matches.size() < 2 ) {
        GRAPHLOG("Match found at: x: %d y: %d type: %s", node->x, node->y, "match4 horizontal");
        matches->match_type =  TableMatch::MATCH_FOUR_HORIZONTAL;
        matches->nodes.insert(matches->nodes.end(), axis_match.horizontal_matches.begin(), axis_match.horizontal_matches.begin()+3);
    }
    
    else if ( matches->match_type == TableMatch::MATCH_THREE && square ) {
        if ( axis_match.vertical_matches.size() <= 2 ) {
            matches->nodes.insert(matches->nodes.end(), axis_match.vertical_matches.begin(), axis_match.vertical_matches.end());
        }
        if (axis_match.horizontal_matches.size() <= 2) {
            matches->nodes.insert(matches->nodes.end(), axis_match.horizontal_matches.begin(), axis_match.horizontal_matches.end());
            GRAPHLOG("Match found at: x: %d y: %d type: %s", node->x, node->y, "match4 square");
        }
        matches->nodes.push_back(square);
        matches->match_type = TableMatch::MATCH_FOUR_SQUARE;
    }
    
    else if ( matches->match_type == TableMatch::MATCH_THREE ) {
        if ( axis_match.vertical_matches.size() == 2 ) {
            GRAPHLOG("Match found at: x: %d y: %d type: %s", node->x, node->y, "match3 vertical");
            matches->nodes.insert(matches->nodes.end(), axis_match.vertical_matches.begin(), axis_match.vertical_matches.end());
        } else if (axis_match.horizontal_matches.size() == 2) {
            GRAPHLOG("Match found at: x: %d y: %d type: %s", node->x, node->y, "match3 horizontal");
            matches->nodes.insert(matches->nodes.end(), axis_match.horizontal_matches.begin(), axis_match.horizontal_matches.end());
        }
    }
    for (auto i : matches->nodes) {
        matches->blocks.push_back(i->getOverBlock());
    }
    matches->source_color = color;
    matches->target_color = color;
    return matches;
}

void TableGraph::checkAdjColors (TableNode* node, std::set<BlockColor>& colors)
{
    if (node->_north &&
        node->_north->getOverBlock()) {
        colors.insert(node->_north->getOverBlock()->getColor());
    }
    if (node->_east &&
        node->_east->getOverBlock()) {
        colors.insert(node->_east->getOverBlock()->getColor());
    }
    if (node->_south &&
        node->_south->getOverBlock()) {
        colors.insert(node->_south->getOverBlock()->getColor());
    }
    if (node->_west &&
        node->_west->getOverBlock()) {
        colors.insert(node->_west->getOverBlock()->getColor());
    }
}

void TableGraph::checkAdjDir(TableNode* node, BlockColor color, TableNode::TableDir dir, std::vector<TableNode*>& matches, bool falling, bool active )
{
    uint8_t counter = 0;
    while (node != nullptr)
    {
        TableNode* target = nullptr;
        switch (dir) {
            case TableNode::NORTH:
                target = node->_north;
                break;
            case TableNode::EAST:
                target = node->_east;
                break;
            case TableNode::SOUTH:
                target = node->_south;
                break;
            case TableNode::WEST:
                target = node->_west;
                break;
            default:
                break;
        }
        if ( target &&
             target->isContainer() &&
             (!active || (active && target->isActive())) &&
             counter < 5 &&
             target->getOverBlock() &&
             target->getOverBlock()->getColor() < num_block_colors &&
             target->getOverBlock()->getColor() == color &&
             target->getOverBlock()->willMatchBlock() &&
             target->getOverBlock()->getFallState() != DESTROYING &&
            
             (target->getOverBlock()->getFallState() != FALLING ||
             (falling && target->getOverBlock()->getFallState() == FALLING &&
              _delegate->blockInNode(target) == target->getOverBlock()))) {
             matches.push_back(target);
            node = target;
             //checkAdjDir(target, color, dir, matches);
            counter++;
        } else {
            node = nullptr;
        }
    }
}

bool TableGraph::hasSpawners()
{
    for (auto& i : _nodes) {
        if (i->isSpawner()) {
            return true;
        }
    }
    return false;
}

void TableGraph::getSpawners(std::vector<TableNode *> &out_spawners, uint8_t opt )
{
    for ( auto& i : _nodes ) {
        if ( i->isSpawner( ) ) {
            TableNodeSpawn* sn = i->getSpawnNode();
            if ((opt == 1 || opt == 3 )&& !sn->willSpawnObjective()) continue;
            if (opt == 2 && sn->willSpawnObjective()) continue;
            if (opt == 3 && sn != i) continue;
            out_spawners.push_back(i);
        }
    }
    std::sort(out_spawners.begin(), out_spawners.end(), [] (TableNode* lhs, TableNode* rhs) -> bool {
        Vec2 lhs_pos = lhs->getTablePosition();
        Vec2 rhs_pos = rhs->getTablePosition();
        
        if (lhs_pos.y >= rhs_pos.y && lhs_pos.x <= rhs_pos.x) return true;
        return false;
    });
}

void TableGraph::getNodesByRegion ( Rect& region, std::vector<TableNode*>& nodes)
{
    for ( uint32_t x = region.origin.x; x < region.origin.x + region.size.width; x++ ) {
        for ( uint32_t y = region.origin.y; y < region.origin.y + region.size.width; y++ ) {
            nodes.push_back(getNodeByPosition(Vec2( x,y )));
        }
    }
}

const std::vector<TableNode*>& TableGraph::getNodes( )    { return _nodes; }
const std::set<TableBlock*>& TableGraph::getOverBlocks( )  { return _over_blocks; }
const std::set<TableBlock*>& TableGraph::getUnderBlocks( )  { return _under_blocks; }

Vec2 TableGraph::getDimensions( )                   { return _graph_dim; }


