//
//  TableBlock.cpp
//  Elixir
//
//  Created by Peter Respondek on 3/20/15.
//
//

#include "../data/TableBlock.h"
#include "../data/TableNode.h"
#include "../data/TableGraph.h"

uint8_t TableBlock::num_block_colors = 6;

void TableBlock::setNode             ( TableNode* node )
{
    if ( _node != node ) {
        _prev_node = _node;
        _node = node;
        if ( checkState(OVER_BLOCK) && _node ) {
            _node->setOverBlock(this);
        } else if ( !checkState(OVER_BLOCK) && _node ) {
            _node->setUnderBlock(this);
        }
    }
    
}

void TableBlock::petrify( bool toggle )
{
    if (willPetrify()) {
        setState(PETRIFIED, toggle);
        if (_delegate) {
            _delegate->petrify( true );
        }
    }
}

TableNode* TableBlock::getPrevNode   ( bool active )
{
    if (active) {
        if (_prev_node && _prev_node->isActive()) {
            return _prev_node;
        }
        return nullptr;
    }
    return _prev_node;
}

TableBlock* TableBlock::createTableBlock(BlockColor color, BlockType type)
{
    TableBlock* block = nullptr;

    switch (type) {
        case MANA: {
            TableBlockMana* mblock = new TableBlockMana();
            mblock->setSourceColor(color);
            mblock->setInvinsible(true);
            return mblock;
        }
            break;
        case OMNI:
            block = new TableBlockOmni();
            break;
        case EYE:
            block = new TableBlockEye();
            break;
        default:
            block = new TableBlock();
            break;
    }
    
    switch (type) {
        default:
            break;
        case COIN:
        case EYE:
        case EGG:
            block->setState(INVINSIBLE);
            break;
    }
    
    block->_block_type = type;
    block->_block_color = color;
    return block;
}

void TableBlock::setFallState( FallState state)
{
    CCASSERT(state != DESTROYING, "use destroy() instead");
    changeState(state);
}

void TableBlock::changeState            ( FallState state )
{
    bool active = true;
    uint8_t inc = 0;
    if ( state == STATIC || state == RESTING || state == DESTROYED ) active = false;
    if ( state != FALLING && state != FALLEN )      _fall_count = 0;
    if ( state == FALLEN && _fall_state != FALLEN ) _fall_count++;
    TableGraph::getInstance()->blockFallStateChanged(this, state,inc);
    _fall_state = state;
    if (_delegate) _delegate->blockFallStateChanged(_fall_state);
    if ( active != checkState(ACTIVE) ) {
        setState(ACTIVE, active);
        if (checkState(ACTIVE) == true)    { TableGraph::getInstance()->increaseFallCount(); inc = 1; }
        else                               { TableGraph::getInstance()->decreaseFallCount(); inc = 2; }
    }
}

uint16_t TableBlockDestructible::getScore()
{
    switch (_block_type) {
        case NORMAL:
        case BLOCKER: {
            switch (_stage) {
                case 0:
                    return 100;
                    break;
                default:
                    return 50;
                    break;
            }
        }
            break;
        case EGG: {
            switch (_stage) {
                case 0:
                    return 1000;
                    break;
                default:
                    return 500;
                    break;
            }
        }
            break;
        default:
            break;
    }
    return 0;
}

uint16_t TableBlock::getScore()
{
    switch (_block_type) {
        case NORMAL:
            return 10;
            break;
        case FRUIT:
        case JELLY:
            return 20;
            break;
        case VERTICAL:
            return 40;
            break;
        case HORIZONTAL:
            return 40;
            break;
        case BLOCKER:
            return 100;
            break;
        case OMNI:
            return 120;
            break;
        case BLAST:
            return 40;
            break;
        case CROSS:
            return 80;
        case EYE:
        case COIN:
            return 1000;
            break;
        case GOAL:
            return 500;
            break;
        default:
            return 0;
            break;
    }
}

bool TableBlock::destroy( uint32_t idx )
{
    if ( !willDestroyBlock() ) return false;
    if ( immunized(idx) ) return false;
    if ( !checkState(PETRIFIED) )
        changeState(DESTROYING);
    return true;
}

bool TableBlockEye::destroy( uint32_t idx )
{
    if (!willDestroyBlock()) { shakeBlock(idx); return false; }
    changeState(DESTROYING);
    return true;
}

bool TableBlockDestructible::destroy( uint32_t idx )
{
    if ( !willDestroyBlock() && shakeBlock(idx) ) { return false; }
    if ( immunized(idx) )         return false;
    if ( !checkState(PETRIFIED) )
        changeState(DESTROYING);
    /*if ( _stage > 0 ) {
        setStage( _stage - 1 );
        return false;
    }*/
    return true;
}

BlockColor   TableBlock::getBlockColorWithChar ( const char x )
{
    
    switch (x) {
        case 'X':
            return BlockColor(rand() % num_block_colors);
            break;
        case 'r':
            return BlockColor::RED;
            break;
        case 'g':
            return BlockColor::GREEN;
            break;
        case 'b':
            return BlockColor::BLUE;
            break;
        case 'o':
            return BlockColor::ORANGE;
            break;
        case 'y':
            return BlockColor::YELLOW;
            break;
        case 'p':
            return BlockColor::PURPLE;
            break;
        case 'w':
            return BlockColor::WHITE;
            break;
        case 'B':
            return BlockColor::BLACK;
            break;
        default: return BlockColor::NONE;
    }

}
BlockType    TableBlock::getBlockTypeWithChar ( const char x, BlockChannel channel )
{
    if (channel == MUTATOR) {
        switch (x) {
            case 'J':
                return BlockType::JELLY;
                break;
            case 'F':
                return BlockType::ICE;
                break;
            default:
                return BlockType::NORMAL;
                break;
        }
    }
    switch (x) {
        default:
            return BlockType::NORMAL;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
            return BlockType::BLOCKER;
            break;
        case '+':
        case ',':
        case '-':
        case '.':
            return BlockType::EGG;
            break;
        case 'v':
            return BlockType::VERTICAL;
            break;
        case 'm':
            return BlockType::MISSLE;
            break;
        case 'f':
            return BlockType::FRUIT;
            break;
        case 'h':
            return BlockType::HORIZONTAL;
            break;
        case 'c':
            return BlockType::CROSS;
            break;
        case 'b':
            return BlockType::BLAST;
            break;
        case '@':
            return BlockType::OMNI;
            break;
        case '*':
            return BlockType::MIRACLE;
            break;
        case 'M':
            return BlockType::MANA;
            break;
        case 'G':
            return BlockType::GOAL;
            break;
        case 'C':
            return BlockType::COIN;
            break;
        case 'E':
            return BlockType::EYE;
            break;
    }

}


TableBlock* TableBlock::createMutatorWithChar(char x, TableBlock* block )
{
    TableBlock* new_block = nullptr;
    
    BlockColor color = getBlockColorWithChar(x);

    switch (x) {
        case 'L':
            block->setState(LOCKED);
            block->setFallState(STATIC);
            new_block = block;
            break;
        case 'S':
            block->petrify();
            new_block = block;
            break;
        case 'f':
        case 'F': {
            TableBlockIce* dblock = new TableBlockIce();
            dblock->setBlockType(ICE);
            dblock->setFallState(STATIC);
            dblock->_prisoner = block;
            if (x == 'f')
                dblock->setStage(1);
            else
                dblock->setStage(0);
            new_block = dblock;
        }
            break;
        case 'R':
        case 'G':
        case 'B':
        case 'Y':
        case 'O':
        case 'P':
        case 'J': {
            TableBlockJelly* dblock = new TableBlockJelly();
            if (color == NONE) {
                dblock->setBlockColor(block->getColor());
            } else {
                dblock->setBlockColor(color);
            }
            dblock->setBlockType(JELLY);
            dblock->setFallState(STATIC);
            dblock->_prisoner = block;
            dblock->setStage(0);
            new_block = dblock;
        }
            break;
    }
    return new_block;
}

TableBlock* TableBlock::createBlockWithChar(char block_type, BlockChannel channel )
{
    TableBlock* block = nullptr;
    //BlockColor color = getBlockColorWithChar(block_type);
    BlockType type = getBlockTypeWithChar(block_type, channel);
    
    switch (channel) {
        case OVERBLOCK_TYPE: {
            switch (block_type) {
                default:
                    block = createTableBlock(NONE,type);
                    break;
                case '0': 
                    return nullptr;
                case '1':
                case '2':
                case '3':
                case '4': {
                    TableBlockDestructible* dblock = new TableBlockDestructible();
                    dblock->setFallState(STATIC);
                    dblock->setBlockType(type);
                    dblock->setStage(uint8_t(block_type - '0') - 1);
                    block = dblock;
                }
                    break;
                case '+':
                case ',':
                case '-':
                case '.': {
                    TableBlockDestructible* dblock = new TableBlockEgg();
                    dblock->setBlockType(type);
                    dblock->setFallState(STATIC);
                    //dblock->setInvinsible(true);
                    dblock->setStage(block_type - '+');
                    block = dblock;
                }
                    break;
            }
        }
            break;
        case UNDERBLOCK: {
            switch (block_type) {
                case 'X':
                    break;
                case '1':
                case '2':
                case '3': {
                    TableBlockDestructible* dblock = new TableBlockDestructible();
                    dblock->setFallState(STATIC);
                    dblock->setStage(uint8_t(block_type - '0') - 1);
                    dblock->setOverBlock(false);
                    block = dblock;
                }
                    break;
            }
        }
            break;
        default:
            CCASSERT(false, "Creating a block with this channel not allowed");
    }
    //if (block_type != 'X') block->setState(IMMUTABLE, true);
    return block;
}
