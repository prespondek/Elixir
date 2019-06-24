//
//  TableAI.hpp
//  Elixir
//
//  Created by Peter Respondek on 6/14/17.
//
//

#ifndef TableAI_hpp
#define TableAI_hpp

class TableHint;
class HUDButton;

#include "../scenes/TableScene.h"
#include "../cocos_extensions/Lanyard_Graph.h"

#ifndef TABLEBOT_DEBUG
#define TABLEBOT_DEBUG 0
#endif

#if !defined (TABLEBOT_DEBUG) || TABLEBOT_DEBUG == 0
#define BOTLOG(...)       do {} while (0)

#elif TABLEBOT_DEBUG == 1
#define BOTLOG(format, ...)      cocos2d::log(format, ##__VA_ARGS__)
#endif

class TableStats : public Node
{
public:
    static TableStats* create( std::string& level );
    bool init ( std::string& level );
    Label* _moves_label;
    Label* _score_label;
    Label* _difficulty_label;
    Label* _winlose_label;
    Label* _level_name_label;
    Label* _hint_label;
};

class TableBot : public TableSceneDelegate
{
public:
    TableBot( TableScene* scene ) :
    TableSceneDelegate (scene),
    _moves(0)
    {  }
    
    static TableBot* create ( TableScene* scene );
    void setStatsLabel( TableStats* stats ) { _stats_lable = stats; }

protected:
    bool init ();
    virtual void tableReady     ( );
    void getGoalDropNodes       ( std::vector<TableNode*>& goals, std::vector<std::vector<TableNode*>>& gnodes );
    void getGoalDropNodes       ( std::vector<TableNode*>& goals, std::vector<TableNode*>& gnodes );
    void getTargetDropNodes     ( std::vector<TableNode*>& goals, std::vector<std::vector<TableNode*>>& gnodes, bool direct = false );
    void getTargetDropNodes     ( std::vector<TableNode*>& goals, std::vector<TableNode*>& gnodes );
    TableHint* getMatchHint     ( );
    TableHint* getDestructHint  ( );
    TableHint* getTargetHint    ( );
    
    /** opt = 0: Drop goal as far as possible.
        opt = 1: Drop fruit from spawner.
        opt = 2: Drop goal as little as possible. */
    TableHint* getDropHint      ( uint8_t opt );
    TableHint* getBlockedHint   ( );
    TableHint* getProximityHint ( );
    TableHint* getClosestHint   ( uint8_t dist_cutoff );
    TableHint* getUnderHint     ( );
    TableHint* getFruitHint     ( );
    TableHint* getPredictiveHint( uint8_t min = 0 );
    TableHint* getMatchFives    ( );
    TableHint* getLowPriorityHint ( );
    
    bool getBoosterHint     ( TableBlock** block = nullptr, TableHint** hint = nullptr, uint8_t opt = 1 );
    bool getSwapHint        ( TableBlock** block = nullptr );
    bool getPaintHint       ( TableBlock** block = nullptr, TableHint** hint = nullptr );
    bool getDispelHint      ( TableBlock** block = nullptr );
    bool getTeleportHint    ( TableBlock** block = nullptr );
    bool getHammerHint      ( TableBlock** block = nullptr, uint8_t opt = 1 );
    
    void getGoals ( std::vector<TableNode*>& out_nodes, bool active = false );
    void getTargets ( std::vector<TableNode*>& out_nodes, bool active = false );
    virtual void setMoves ( uint16_t moves );
    virtual uint16_t gameOver ( std::string& str, uint32_t points, bool success );
    virtual void begin ( std::string& level_name );
    void getHints ( );
    void getMatchNodes ( std::vector<TableNode*>& out_nodes, TableMatch* match );
    void getShakeNodes ( std::vector<TableNode*>& out_nodes );
    void getBlockedNodes ( std::vector<TableNode*>& out_nodes );

    
    std::vector<std::vector<TableNode*>> _hint_intersect;
    std::vector<HUDButton*> _boosters;
    std::vector<TableHint*> _hints;
    std::vector<TableNode*> _goals;
    Lanyard::Dijkstra<TableNode*> _graph;
    ValueMap _level_stats;
    
    uint16_t _moves;
    
    TableNode* booster_hint_1 = nullptr;
    TableNode* booster_hint_2 = nullptr;
    
    TableStats* _stats_lable = nullptr;
    
};

#endif /* TableAI_hpp */

inline void TableBot::setMoves ( uint16_t moves ) {
    _moves = moves;
}
