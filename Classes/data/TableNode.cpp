//
//  TableNode.cpp
//  Elixir
//
//  Created by Peter Respondek on 3/20/15.
//
//

#include "../data/TableNode.h"
#include "../data/TableGraph.h"

bool TableNodeDelegate::hasTouchBlock()
{
    CCASSERT(_node, "No node assigned");
    return _node->hasTouchBlock();
}

TableNode* TableNodeDelegate::getNode() const
{
    return _node;
}

void TableNodeDelegate::updateNode()
{
    _node->updateNode();
}

TableNodeSpawn* TableNode::getSpawnNode( )
{
    TableNode* prev = this;
    while ( prev ) {
        if ( prev->getType( ) == SPAWN )
            return static_cast<TableNodeSpawn*>( prev );
        prev = prev->getPointNode( );
    }
    return nullptr;
}

TableNodeSpawn* TableNode::getSpawnerNode( )
{
    if ( isContainer( ) ) {
        if ( this->getType( ) == SPAWN && this->isActive() )
            return static_cast<TableNodeSpawn*>(this);
        bool target_a = false;
        bool target_b = false;
        
        TableNode* next = getNextNode();
        TableNode* prev = getPointNode();
        if (next && next->isActive() && !this->isActive()) {
            target_b = true;
        }
        if (prev && this->isActive() && !prev->isActive())
            target_a = true;
        
        TableNodeSpawn* sn = getSpawnNode();
        if (sn && ((target_a && sn->isTile()) ||
            (target_b && !sn->isTile())))
            return sn;
    }
    return nullptr;
    /*if ( isContainer( ) && ( isActive( ) || getSpawner( ) ) ) {
     if ( this->getType( ) == SPAWN )
     return static_cast<TableNodeSpawn*>(this);
     
     TableNode* prev = getPrevNode();
     while ( prev ) {
     if ( prev->isActive( ) ) return nullptr;
     if ( prev->getType( ) == SPAWN )
     return static_cast<TableNodeSpawn*>( prev );
     prev = prev->getPrevNode( );
     }
     }
     return nullptr;*/
}

void TableNode::updateNode()
{
    GRAPHLOG("Checking node %p at: x: %d y: %d ", this, x, y);
    if (getOverBlock() && getOverBlock()->getFallState() == FALLING) {
        getOverBlock()->setFallState(FALLEN);
    }
    else if (getOverBlock() && getOverBlock()->getFallState() == STOPPING) {
        getOverBlock()->setFallState(STOPPED);
    }
    evaluateNode( );
    //TableGraph::getInstance()->checkGraph();
}

bool TableSpawn::popSpawn ( BlockColor& color, BlockType& type )
{
    if (_pending_spawn) {
        color = _next_color; type = _next_type;
        _pending_spawn = false;
        //TableGraph::getInstance()->blockSpawned( this );
        return true;
    }
    return false;
}

void TableNode::getAdjacentNodes  ( std::vector<TableNode*>& out_nodes, std::function<bool(TableNode*)> func ) const
{
    for ( uint8_t i = 0; i <= 6; i+=2 ) {
        TableNode* node = getAdjacentNode( TableNode::TableDir(i), func);
        if (node)
            out_nodes.push_back(node);
    }
}

TableNode* TableNode::getAdjacentNode  ( TableDir dir, std::function<bool(TableNode*)> func ) const
{
    TableNode* adj = nullptr;
    switch (dir) {
        case NORTH:
            adj = _north;
            break;
        case NORTH_EAST:
            adj = _north_east;
            break;
        case EAST:
            adj = _east;
            break;
        case SOUTH_EAST:
            adj = _south_east;
            break;
        case SOUTH:
            adj = _south;
            break;
        case SOUTH_WEST:
            adj = _south_west;
            break;
        case WEST:
            adj = _west;
            break;
        case NORTH_WEST:
            adj = _north_west;
            break;
    }
    if (func) {
        if (adj && func(adj)) return adj;
    } else {
        return adj;
    }
    return nullptr;
}

bool TableNode::isAdjacent(TableNode *node)
{
    if ( node == this ) return false;
    if ( node == _north || node == _south || node == _east || node == _west ) {
        return true;
    }
    return false;
}

TableNode* TableNode::getNextNode( std::function<bool(TableNode*)> func ) const
{
    return getAdjacentNode(_node_dir, func);
}

TableNode* TableNodePipe::getNextNode( std::function<bool(TableNode*)> func ) const
{
    TableNode* next = TableNode::getNextNode(func);
    if (!next)
        next = _exit;
        if (func && _exit && !func(_exit)) next = nullptr;
    return next;
}

void TableNodePipe::setOverBlock                ( TableBlock* block )
{
    TableNode::setOverBlock(block);
}


TableNode::TableDir TableNode::getInverseDirection (TableNode::TableDir dir)
{
    switch (dir) {
            
        case NORTH:
            return SOUTH;
            break;
        case NORTH_EAST:
            return SOUTH_WEST;
            break;
        case EAST:
            return WEST;
            break;
        case SOUTH_EAST:
            return NORTH_WEST;
            break;
        case SOUTH:
            return NORTH;
            break;
        case SOUTH_WEST:
            return NORTH_EAST;
            break;
        case WEST:
            return EAST;
            break;
        case NORTH_WEST:
            return SOUTH_EAST;
            break;
    }
}


TableNode* TableNode::getPrevEmptyNode( )
{
    return getPrevNode([] (TableNode* node) {
        return ( node->isContainer() || node->getType() == PIPE );
    });
}

TableNode* TableNode::getPointEmptyNode( )
{
    return getPointNode([] (TableNode* node) {
        return ( node->isContainer() || node->getType() == PIPE );
    });
}

TableNode* TableNodePipe::getPointNode( std::function<bool(TableNode*)> func ) const
{
    TableNode* next = _entry;
    if (next && this == _entry) {
        next = next->TableNode::getPointNode(func);
    }
    else if (next && func && !func(next))
        return nullptr;
    return next;
}

TableNode* TableNodePipe::getPrevNode( std::function<bool(TableNode*)> func ) const
{
    return getPointNode();
}

TableNode* TableNode::getPointNode( std::function<bool(TableNode*)> func ) const
{
    if (!func) func = [] (TableNode* node) { return true; };
    TableNode* adj = getAdjacentNode(getInverseDirection(_node_dir),func);
    if (adj && adj->getDirection() == _node_dir && func(adj) ) return adj;
    
    for ( uint8_t i = 0; i <= 6; i+=2) {
        TableDir dir = TableDir(i);
        if (dir == _node_dir) continue;
        adj = getAdjacentNode( dir, func );
        if (!adj || !func(adj)) continue;
        switch (dir) {
            case NORTH:
                if ( adj->getDirection() == SOUTH )
                    return adj;
                break;
            case EAST:
                if ( adj->getDirection() == WEST )
                    return adj;
                break;
            case SOUTH:
                if ( adj->getDirection() == NORTH )
                    return adj;
                break;
            case WEST:
                if ( adj->getDirection() == EAST )
                    return adj;
                break;
            default:
                break;
        }
    }
    return nullptr;
}

TableNode* TableNode::getPrevNode( std::function<bool(TableNode*)> func ) const
{
    TableNode* adj = getPointNode(func);
    if (adj)
        return adj;
    
    if (!func) func = [] (TableNode* node) { return true; };
    for ( uint8_t i = 1; i <= 8; i+=2) {
        TableDir dir = TableDir(i);
        if (dir == _node_dir) continue;
        adj = getAdjacentNode( dir, func );
        if (!adj || !func(adj)) continue;
        TableDir adj_dir = adj->getDirection();
        switch (dir) {
                
            case NORTH_EAST:
                if ( adj_dir == SOUTH || adj_dir == WEST )
                    return adj;
                break;
            case NORTH_WEST:
                if ( adj_dir == SOUTH || adj_dir == EAST )
                    return adj;
                break;
            case SOUTH_EAST:
                if ( adj_dir == NORTH || adj_dir == WEST )
                    return adj;
                break;
            case SOUTH_WEST:
                if ( adj_dir == EAST || adj_dir == EAST )
                    return adj;
                break;
            default:
                break;
        }
    }
    return nullptr;
}

void TableNodePipe::evaluateNode( )
{
    TableGraph* graph = TableGraph::getInstance();
    if ( _entry == nullptr ) return;
    
    _entry->getPrevEmptyNode()->evaluateNode();

    TableBlock* block = _entry->getOverBlock();
    
    if (!block ||
        block->getFallState() == STATIC ||
        block->getFallState() == FALLING ||
        block->getFallState() == STOPPING ||
        block->getFallState() == DESTROYING ) {
        return;
    }
    
    graph->swapBlocks(_entry,_exit);
    graph->resetBlock(block);
    _exit->TableNode::evaluateNode();
    CCASSERT(block->getFallState() != STOPPING, "block stopped");
}

BlockColor TableNodeSpawn::getNextColor()
{
    uint8_t color = rand() % (TableBlock::num_block_colors);
    if ( _color_count >= 2 ) {
        color = rand() % (TableBlock::num_block_colors - 1);
        if (color == uint8_t(_prev_color)) color++;
        _color_count = 0;
    }
    return BlockColor(color);
}

void TableNodeSpawn::setOverBlock (TableBlock* block) {
    TableNode::setOverBlock(block);
    if (block) {
        if ( block->getColor() == _prev_color ) {
            _color_count++;
        } else {
            _prev_color = block->getColor();
            _color_count = 0;
        }
        if ( block->getType() == _prev_type ) {
            _type_count++;
        } else {
            _prev_type = block->getType();
            _type_count = 0;
        }
    }
}

void TableNode::evaluateNode( )
{
    TableGraph* graph = TableGraph::getInstance();
    
    // if I'm a spawn node and the node beneath is empty then spawn a new block
    if ( getType() == SPAWN && !getOverBlock() ) {
        TableNodeSpawn* spawn = static_cast<TableNodeSpawn*>(this);
        TableBlock* block = nullptr;
        if (_spawner) {
            block = graph->spawnBlock(this, block);
        }
        if (!block) {
            block = graph->createTableBlock(spawn->getNextColor(),spawn->getNextType());
        }
        graph->emplaceBlock(this, block);
    }

    // if I'm a destructible block take damage
    if (checkState(SHAKEN)) {
        TableBlock* over_block = graph->_delegate->blockInNode(this);
        graph->shakeBlock( over_block, this, _shake_idx );
        changeState(SHAKEN,false);
    }
    if (checkState(FROZEN)) {
        return;
    }
    
    TableBlock* block = getOverBlock();

    // ignore any other state except resting or fallen
    if (!block || 
        block->getFallState() == STATIC ||
        block->getFallState() == FALLING ||
        block->getFallState() == STOPPING ||
        block->getFallState() == DESTROYING ) {
        return;
    }

    // at this point we have filter out everything except for normal blocks ready to be fall
    TableNode* target = getEmptyDropNode();
    
    // because higher blocks can fall faster than lower blocks we need to check for collisions
    if (target && target->getDelegate() && graph->_delegate ) {
        TableNode* exit = target->getPortalNode();
        TableBlock* target_block = graph->_delegate->blockInNode(exit);
        if ( target_block && block->getFallCount() > target_block->getFallCount() ) {
            graph->stopBlock(this);
            return;
        }
    }
    
    // we found a node to make our block fall into so drop the block
    if ( target && target->filterBlockType(getOverBlock()->getType()) ) {
        if ( target->isSpawner() || getType() == SPAWN ) {
            uint8_t fcount = block->getFallCount();
            if ( block->getColor() == graph->getFruitColor() && block->getType() == BlockType::NORMAL ) {
                block = graph->createTableBlock( getOverBlock()->getColor(),BlockType::FRUIT );
                graph->emplaceBlock(this, block);
            }
            block->setFallCount(fcount);
        }
        if ( target->isSpawner() ) {
            TableBlock* new_block = graph->spawnBlock(this, block);
            if (new_block) {
                block = new_block;
                graph->blockDelegateToNode (block, this);
            }
        }
        if (getType() == SPAWN) {
            TableNode* spawn_node = target->getNextNode();
            if (spawn_node && spawn_node->getOverBlock()) {
                block->setFallCount(spawn_node->getOverBlock()->getFallCount());
            }
        }
        graph->dropBlock(block, this, target);
        
    // we havn't unable to find a node to make our block fall into so stop the block
    } else {
        graph->stopBlock(this);
    }
}

bool TableNode::cavityCheck ()
{
    if ( getType() == SPAWN || (getOverBlock() && getOverBlock()->getFallState() != STATIC) )
        return false;
    else if ( !isContainer() || (getOverBlock() && getOverBlock()->getFallState() == STATIC) )
        return true;
    switch (_node_dir) {
        case NORTH:
            if ( _south && _south->_node_dir == NORTH )   { return _south->cavityCheck(); }
            if ( _east  && _east->_node_dir == WEST )     { return _east->cavityCheck(); }
            if ( _west  && _west->_node_dir == EAST )     { return _west->cavityCheck(); }
            break;
        case EAST:
            if ( _west  && _west->_node_dir == EAST )     { return _west->cavityCheck(); }
            if ( _north && _north->_node_dir == SOUTH )   { return _north->cavityCheck(); }
            if ( _south && _south->_node_dir == NORTH )   { return _south->cavityCheck(); }
            break;
        case SOUTH:
            if ( _north && _north->_node_dir == SOUTH )   { return _north->cavityCheck(); }
            if ( _east  && _east->_node_dir == WEST )     { return _east->cavityCheck(); }
            if ( _west  && _west->_node_dir == EAST )     { return _west->cavityCheck(); }
            break;
        case WEST:
            if ( _east  && _east->_node_dir == WEST )     { return _east->cavityCheck(); }
            if ( _north && _north->_node_dir == SOUTH )   { return _north->cavityCheck(); }
            if ( _south && _south->_node_dir == NORTH )   { return _south->cavityCheck(); }
            break;
        default:
            CCASSERT(false, "unsupported node dir");
            break;
    }
    return true;
}

TableNode* TableNode::getAdjacentDropNode ( const std::function<bool(TableNode*,TableBlock*)>& filter1, TableBlock* block  )
{
    TableNode* target [2] = {nullptr,nullptr};
    switch (_node_dir) {
        case NORTH:
            target[0] = _north_east; target[1] = _north_west;
            break;
        case EAST:
            target[0] = _north_east; target[1] = _south_east;
            break;
        case SOUTH:
            target[0] = _south_east; target[1] = _south_west;
            break;
        case WEST:
            target[0] = _north_west; target[1] = _south_west;
            break;
        default:
            CCASSERT(false, "unsupported node dir");
            break;
    }
    for ( uint8_t i = 0; i < 2; i++ ) {
        if (target[i] && !filter1(target[i],block))
            target[i] = nullptr;
    }
    if (target[0] && target[1]) {
        return target[rand() % 2];
    }
    else if (target[0]) return target[0];
    else if (target[1]) return target[1];
    return nullptr;
}

bool TableNode::isAdjacentDropNode( TableNode* target, TableBlock* block )
{
    return (target && block &&
            willDiagonalDrop( ) &&
            target->willDiagonalFill( ) &&
            target->cavityCheck( )  &&
            target->filterBlockType(block->getType( )));
}

bool TableNode::isAdjacentEmptyDropNode( TableNode* target, TableBlock* block )
{
    return isAdjacentDropNode(target, block) && !target->getOverBlock();
}

bool TableNode::isEmptyDropNode( TableNode* target,TableBlock* block )
{
    return isDropNode( target, block ) && !target->getOverBlock() && !target->getPortalNode()->getOverBlock();
}

bool TableNode::isDropNode(  TableNode* target,TableBlock* block  )
{
    return ( block && target && target->getType() != SPAWN && target->filterBlockType(block->getType()) );
}

TableNode* TableNode::getEmptyDropNode()
{
    TableBlock* block = getOverBlock();
    TableNode* target = getDropNode( CC_CALLBACK_2(TableNode::isEmptyDropNode, this),
                                     CC_CALLBACK_2(TableNode::isAdjacentEmptyDropNode, this), block  );
    return target;
}

TableNode* TableNode::getDropNode( const std::function<bool(TableNode*,TableBlock*)>& filter1,
                                   const std::function<bool(TableNode*,TableBlock*)>& filter2, TableBlock* block ) {
    TableNode* target = getNextNode();
    if ( filter1 ( target, block ) )
        return target;
    else {
        target = getAdjacentDropNode( filter2, block );
    }
    return target;
}

void TableNode::setState  ( NodeState state, bool set)
{
    if (!set) _node_state &= ~state;
    else _node_state |= state;
}

void TableNode::changeState  ( NodeState state, bool set)
{
    TableGraph* graph = TableGraph::getInstance();
    
    if ( !checkState(ACTIVE) || !isContainer() ) return;
    
    if ( state == FROZEN && !set && _freeze_count > 0 ) {
        _freeze_count--;
    }
    
    if ( state == FROZEN && set ) {
        _freeze_count++;
    }
    
    int inc = 0;
    bool was_frozen = checkState(FROZEN);
    bool was_shaken = checkState(SHAKEN);
    
    if ( (was_frozen && state == FROZEN && _freeze_count > 0) || checkState(state) == set )     {
        graph->nodeStateChanged(this, state, set, inc);
        return;
    }

    if ( !was_frozen && !was_shaken && set ) {
        graph->increaseFallCount();
        inc = 1;
    }
    
    setState(state, set);
    
    if ( _delegate ) _delegate->stateSet( state);
    if ( was_frozen ) evaluateNode();
    else if ( was_shaken ) _shake_idx = 0;
    
    if ( (was_frozen || was_shaken) && !checkState(FROZEN) && !checkState(SHAKEN)) {
        graph->decreaseFallCount();
        inc = 2;
    }
    graph->nodeStateChanged(this, state, set, inc);
}

void TableNode::shake( uint32_t idx )
{
    //CCASSERT(_shake_idx == 0, "shake idx is already set and has not been processed yet");
    _shake_idx = idx;
    changeState(SHAKEN,true);
}

bool TableNode::isContainer                ( ) const
{ return true; }
bool TableNode::hasTouchBlock              ( )
{ if (!isActive()) return false; return true; }
bool TableNode::isSpawner                  ( )
{ return _spawner; }

void TableNode::blockDestroyed                 ( TableBlock* block )
{

}

void TableNode::setOverBlock                ( TableBlock* block )
{
    CCASSERT(!_over_block || (_over_block && _over_block->getFallState() != FALLING), "");
    if ( _over_block != block ) {
        _over_block = block;
        if ( _over_block ) _over_block->setNode(this);
    }
}

void TableNode::setUnderBlock               ( TableBlock* block )
{
    if ( _under_block != block ) {
        _under_block = block;
        if ( _under_block ) _under_block->setNode(this);
    }
}
