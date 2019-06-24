//
//  TableBlock.h
//  Elixir
//
//  Created by Peter Respondek on 3/20/15.
//
//

#ifndef __Elixir__TableBlock__
#define __Elixir__TableBlock__

#include <stdint.h>
#include <string>
#include <set>

class TableNode;
class TableBlock;
class TableBlockDelegate;

enum BlockColor : char {
    RED,
    GREEN,
    BLUE,
    YELLOW,
    ORANGE,
    PURPLE,
    BLACK,
    WHITE,
    NONE
};
const uint8_t num_block_colors = 6;

enum BlockType : char {
    NORMAL,
    JELLY,
    FRUIT,
    VERTICAL,
    CROSS,
    MISSLE,
    HORIZONTAL,
    OMNI,
    BLAST,
    EYE,
    COIN,
    GOAL,
    EGG,
    BLOCKER,
    STONE,
    MIRACLE,
    MANA,
    ICE
};

enum BlockChannel : char {
    OVERBLOCK_COLOR,
    OVERBLOCK_TYPE,
    UNDERBLOCK,
    MUTATOR

};

enum FallState : char {
    STATIC,
    RESTING,
    FALLING,
    FALLEN,
    STOPPING,
    STOPPED,
    DESTROYING,
    DESTROYED
};

enum BlockState {
    ACTIVE              = 0x01,
    OVER_BLOCK          = 0x02,
    INVINSIBLE          = 0x04,
    IMMUTABLE           = 0x08,
    PETRIFIED           = 0x10,
    LOCKED              = 0x20
};


class TableBlockDelegate {
    friend class TableBlock;
public:
    uint8_t getFallCount();
    virtual void setBlock           ( TableBlock* block ) { _block = block; }
    virtual void transmute          ( BlockColor color, BlockType type ) {}
    virtual void blockInserted      ( ) {}
    virtual void blockSelected      ( bool selected ) {}
    virtual void shake              ( ) {}
    virtual void petrify            ( bool animate ) {}
    virtual void freeze             ( ) {}
    virtual bool destroyDelegate    ( ) { return true; }


protected:
    virtual void blockFallStateChanged ( FallState state ) {}
    TableBlock* _block = nullptr;
};

class TableBlock
{
    friend class TableBlockDelegate;
    friend class TableGraph;
    
public:
    
    static uint8_t num_block_colors;
    
    virtual void setFallState       ( FallState state );
    inline void setOverBlock        ( bool flag ) { setState(OVER_BLOCK, flag); }
    inline bool isOverBlock         ( ) { return checkState(OVER_BLOCK); }
    virtual uint16_t getScore       ( );
    /** if this is set to true when the block is shaken it will damage the block */
    virtual bool willBlockShake     ( ) { return checkState(PETRIFIED);  }
    /** if this is set to true when this block is destoryed it will shake adjacent nodes */
    virtual bool willShakeBlock     ( ) { return !checkState(PETRIFIED); }
    /** will be destroyed if shook */
    virtual bool willShakeDestroy   ( ) { return false; }
    virtual bool willFreeze         ( ) { return true; }
    virtual bool willPetrify        ( ) {
        switch (_block_type) {
            case NORMAL:
            case FRUIT:
            case VERTICAL:
            case CROSS:
            case MISSLE:
            case HORIZONTAL:
            case OMNI:
            case BLAST:
                return true;
                break;
        }
        return false;
    }
    /** if under blocks will be destroyed if the block is destroyed */
    virtual bool willDestroyUnderBlock( ) { return !checkState(PETRIFIED); }
    virtual bool willMatchBlock     ( ) { return !checkState(PETRIFIED) && (_block_color < num_block_colors ||
                                                                            _block_type == OMNI || _block_type==MIRACLE ||_block_type==MANA) &&
       _fall_state != DESTROYING && _fall_state != DESTROYED; }
    virtual bool willMoveBlock      ( ) { return _fall_state != STATIC && !checkState(PETRIFIED); }
    virtual bool willDestroyBlock   ( ) { return !(isInvinsible() ||
                                                   _fall_state == DESTROYING ||
                                                   _fall_state == DESTROYED); }
    virtual void blockSelected      ( bool selected ) { _delegate->blockSelected(selected); }
    virtual bool shakeBlock         ( uint32_t idx ) {
        if (immunized(idx)) return false;
        if (checkState(PETRIFIED)) { petrify(false); }
        else { _delegate->shake();} return true; }
    
    inline bool checkState          ( BlockState state )
    { return _block_state & state; }
    inline bool immunized           ( uint32_t idx ) { return idx != 0 && !_destuct_ids.insert(idx).second; }
    inline void setState            ( BlockState state, bool set = true )
    { if (!set) _block_state &= ~state; else _block_state |= state; }
    inline void clearState          ( BlockState state ) { setState(state, false); }
    
    TableBlockDelegate* getDelegate ( ) { return _delegate; }
    void setDelegate                ( TableBlockDelegate* delegate )
    { _delegate = delegate; if (delegate) delegate->setBlock(this); }
    void clearDelegate              ( )
    { if (_delegate) _delegate->_block = nullptr; _delegate = nullptr; }
    inline BlockColor getColor      ( )
    { return _block_color; }
    inline BlockType getType        ( )
    { return _block_type; }
    inline FallState getFallState       ( )
    { return _fall_state; }
    inline virtual void setBlockColor    ( BlockColor color )
    { _block_color = color; }
    inline virtual void setBlockType     ( BlockType type )
    { _block_type = type; }
    inline void setFallCount        ( uint8_t count )
    { _fall_count = count; }
    inline uint8_t getFallCount     ( )
    { return _fall_count; }
    inline void setInvinsible       ( bool invinsible )
    {
        setState(INVINSIBLE, invinsible);
    }
    inline bool isInvinsible        ( )
    { return checkState(INVINSIBLE); }
    inline bool isPetrified         ( )
    { return checkState(PETRIFIED); }
    inline bool isObjective         ( )
    { return TableBlock::isObjective(_block_type); }
    virtual bool destroy            ( uint32_t idx );
    static bool isObjective         ( BlockType type )
    {   switch (type) {
            case COIN:
            case GOAL:
            case EYE:
            case EGG:
            return true;
            break;
        default:
            break;
    }
        return false;
    }
    void setNode                    ( TableNode* node );
    void petrify                    ( bool toggle = true );

    inline TableNode* getNode       ( )
    { return _node; }
    TableNode* getPrevNode          ( bool active = true );
    void getColor(std::string& color_out) {
        switch (_block_color) {
            case RED: color_out = "Red"; break;
            case GREEN: color_out = "Green"; break;
            case BLUE: color_out = "Blue"; break;
            case YELLOW: color_out = "Yellow"; break;
            case ORANGE: color_out = "Orange"; break;
            case PURPLE: color_out = "Purple"; break;
            default: color_out = "None"; break;
        }
    }
    
    static BlockColor  getBlockColorWithChar ( const char x );
    static BlockType   getBlockTypeWithChar ( const char x, BlockChannel channel );
    
protected:
    static TableBlock* createTableBlock ( BlockColor color = NONE, BlockType type = NORMAL );
    static TableBlock* createBlockWithChar (char x, BlockChannel channel = OVERBLOCK_TYPE);
    static TableBlock* createMutatorWithChar (char x, TableBlock* block );

    TableBlock() :
    _block_color    (NONE),
    _block_type     (NORMAL),
    _fall_state     (RESTING),
    _delegate       (nullptr),
    _fall_count     (0),
    _node           (nullptr),
    _prev_node      (nullptr),
    _block_state    (0)
    
    { setOverBlock(true); }
    void changeState                ( FallState state );

    uint8_t _fall_count;
    uint8_t _block_state;
    BlockColor _block_color;
    BlockType  _block_type;
    FallState _fall_state;
    TableBlockDelegate* _delegate;
    TableNode* _node;
    TableNode* _prev_node;
    std::set<uint32_t> _destuct_ids;

};

// destructible blocks have multiple stages before their destruction. You might think of this as health.
class TableBlockDestructible : public TableBlock
{
    friend class TableGraph;
    friend class TableBlock;
public:
    virtual bool        willShakeBlock         ( ) { return  _stage == 0 && _fall_state == DESTROYING; }
    virtual bool        willBlockShake         ( ) { return true; }
    virtual bool        willDestroyUnderBlock  ( ) { return true; }
    virtual bool        willFreeze       ( ) { return false; }
    virtual bool        willPetrify      ( ) { return false; }
    virtual bool        destroy          ( uint32_t idx );
    virtual uint16_t    getScore         ( );
    inline uint8_t      getStage         ( ) { return _stage; }
    virtual bool        willShakeDestroy ( ) { return true; }
    virtual bool        willDestroyBlock ( ) { return TableBlock::willDestroyBlock() && _stage == 0; }
    virtual bool        shakeBlock       ( uint32_t idx ) {
        if (_stage == 0 && !checkState(PETRIFIED)) return false;
        if (immunized(idx)) return false;
        if (checkState(PETRIFIED)) { petrify(false); return true; }
        if ( _stage > 0 ) {
            _stage -= 1;
           _delegate->shake();
            return true;
        }
        return false;
    }
    
protected:
    inline void setStage ( uint8_t stage ) {
        //CCLOG("Set Block Stage %p: %d", this, stage);
        _stage = stage; }
    uint8_t _stage;
    TableBlockDestructible() : _stage (0)
    { }
};

class TableBlockOmni : public TableBlock
{
    friend class TableGraph;
    friend class TableBlock;
public:
    virtual bool willFreeze         ( ) { return false; }
    virtual bool willPetrify         ( ) { return false; }
    inline virtual void setBlockColor    ( BlockColor color )
    {  }
    inline virtual void setBlockType     ( BlockType type )
    {  }
    inline BlockColor getTargetColor    ( )
    { return _target_color; }
    inline BlockType getTargetType     ( )
    { return _target_type; }
    inline void setTargetColor    ( BlockColor color )
    { _target_color = color; }
    inline void setTargetType     ( BlockType type )
    { _target_type = type; }
    
protected:
    TableBlockOmni() :
    _target_color(BlockColor::NONE),
    _target_type(BlockType::NORMAL)
    {
        _block_color = BlockColor::NONE;
        _block_type = BlockType::OMNI;
    }
    
    BlockColor _target_color;
    BlockType _target_type;
};

class TableBlockMana : public TableBlockOmni
{
    friend class TableGraph;
    friend class TableBlock;
public:
    BlockColor  getSourceColor  ( )                     { return _source_color; }
    void        setSourceColor  ( BlockColor color )    { _source_color = color; }
    uint8_t     getBlockCount   ( )                     { return _block_count; }
    void        setBlockCount   ( uint8_t num )         { _block_count = num; }
    
    uint8_t stage = 0;

protected:
    TableBlockMana() {
        _block_color = BlockColor::NONE;
        _block_type = BlockType::MANA;
    }
    uint8_t _block_count = 0;
    BlockColor _source_color;
};

class TableBlockPrison : public TableBlockDestructible
{
    friend class TableGraph;
    friend class TableBlock;
    friend class TableBlockSprite;
public:
    TableBlock* getPrisoner ( )         { return _prisoner; }
    virtual bool willShakeBlock ( )     { return false; }
    virtual bool willBlockShake ( )     { return false; }
    virtual bool willShakeDestroy ( )   { return false; }
    virtual bool willFreeze ( )         { return false; }
    virtual bool willPetrify ( )        { return false; }

    void setPrisoner ( TableBlock* block ) { _prisoner = block; }

protected:
    
    TableBlockPrison ():
    _prisoner(nullptr) {}
    
    TableBlock* _prisoner;
    
};

class TableBlockJelly : public TableBlockPrison
{
public:
    virtual void setBlockColor    ( BlockColor color )
    { _block_color = color;
        if (_prisoner) {
            _prisoner->TableBlock::setBlockColor(color);
        }
    }
    
};

class TableBlockPrisonShakeable : public TableBlockPrison
{
public:
    virtual bool willBlockShake     ( ) { return true; }
    virtual bool willShakeDestroy   ( ) { return true; }
};

class TableBlockIce : public TableBlockPrisonShakeable
{
public:
    virtual bool willMatchBlock     ( ) { return false; }
};

class TableBlockEgg : public TableBlockDestructible
{
public:
    virtual bool willBlockShake ( ) { return false; }
    virtual bool willShakeDestroy( ) { return false; }
    virtual bool willFreeze ( ) { return false; }
    virtual bool willPetrify ( ) { return false; }
    
};

class TableBlockEye : public TableBlock
{
    friend class TableGraph;
    friend class TableBlock;
public:
    virtual bool willShakeBlock         ( ) { return false; }
    virtual bool willDestroyUnderBlock  ( ) { return false; }
    virtual bool destroy                ( uint32_t idx );
    virtual bool willFreeze             ( ) { return false; }
    virtual bool willPetrify            ( ) { return false; }
    virtual bool willMatchBlock         ( ) { return false; }
    virtual bool willMoveBlock          ( ) { return false; }
    virtual bool willBlockShake         ( ) { return ( TableBlock::willBlockShake() &&
                                               _fall_state == RESTING ); }
    bool shakeBlock                     ( uint32_t idx ) { if ( !_enraged && !_livid && TableBlock::shakeBlock(idx)) { _enraged = true; return true;  } return false; }
    void enrage                         ( bool calm ) { _enraged = calm; }
    void livid                          ( bool calm ) { if ( !_livid && calm ) {_livid = calm; _enraged = false; _delegate->shake();}
         _livid = calm; }
    bool isEnraged                      ( ) { return _enraged; }
    bool isLivid                        ( ) { return _livid; }

protected:
    bool _enraged = false;
    bool _livid = false;
};

#endif /* defined(__Elixir__TableBlock__) */
