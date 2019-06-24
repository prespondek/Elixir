//
//  TableData.h
//  Elixir
//
//  Created by Peter Respondek on 6/18/15.
//
//

#ifndef __Elixir__TableData__
#define __Elixir__TableData__

#include "cocos2d.h"

USING_NS_CC;

enum TableGoal {
    GOAL_UNDERBLOCKS,
    GOAL_DISPEL,
    GOAL_COLLECT,
    GOAL_MATCH,
    GOAL_JUICE,
    GOAL_ICE,
    GOAL_FRUIT,
    GOAL_SCORE_NOFRUIT,
    GOAL_SCORE
};

class TableData : public Ref
{
public:
    CREATE_FUNC(TableData);
    
    ValueMap& getLevelData(uint16_t level_idx);
    ValueMap& getLevelData(const std::string& level);
    ValueMap& getTableData();
    TableGoal getLevelRule(const std::string& level);
    TableGoal getLevelRule(uint16_t level_idx);
    uint8_t getLevelStars (uint16_t level_idx, uint32_t score);
    
    static Sprite* getLevelRuleSpite (TableGoal rule);
    static std::string getLevelRuleText (TableGoal rule);

protected:
    TableData();
    bool init ();
    ValueMap _levels_data;

};

inline ValueMap& TableData::getTableData() { return _levels_data; }

#endif /* defined(__Elixir__TableData__) */
