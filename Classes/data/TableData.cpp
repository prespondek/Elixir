//
//  TableData.cpp
//  Elixir
//
//  Created by Peter Respondek on 6/18/15.
//
//

#include "../data/TableData.h"
#include "../cocos_extensions/Lanyard_Util.h"
#include "../data/ElixirLocalization.h"


TableData::TableData()
{
}

Sprite* TableData::getLevelRuleSpite (TableGoal rule)
{
    switch (rule) {
        case GOAL_UNDERBLOCKS:
            return Sprite::createWithSpriteFrameName("goal_caramel.png");
        case GOAL_DISPEL:
            return Sprite::createWithSpriteFrameName("goal_dispel.png");
        case GOAL_ICE:
            return Sprite::createWithSpriteFrameName("goal_ice.png");
        case GOAL_COLLECT:
            return Sprite::createWithSpriteFrameName("goal_coins.png");
        case GOAL_JUICE:
            return Sprite::createWithSpriteFrameName("goal_juice.png");
        case GOAL_FRUIT:
            return Sprite::createWithSpriteFrameName("goal_fruit.png");
        case GOAL_MATCH:
            return Sprite::createWithSpriteFrameName("goal_egg.png");
        case GOAL_SCORE_NOFRUIT:
            return Sprite::createWithSpriteFrameName("goal_score.png");
        case GOAL_SCORE:
            return Sprite::createWithSpriteFrameName("goal_score.png");

    }
    return nullptr;
}

std::string TableData::getLevelRuleText(TableGoal rule)
{
    switch (rule) {
        case GOAL_UNDERBLOCKS:
            return LOCALIZE("Crack the Caramel");
        case GOAL_DISPEL:
            return LOCALIZE("Dispel the Gorgon's Eye");
        case GOAL_ICE:
            return LOCALIZE("Break the Ice");
        case GOAL_COLLECT:
            return LOCALIZE("Drop the Coins");
        case GOAL_JUICE:
            return LOCALIZE("Make Juice");
        case GOAL_FRUIT:
            return LOCALIZE("Match the Fruit");
        case GOAL_MATCH:
            return LOCALIZE("Release the Phoenix");
        case GOAL_SCORE_NOFRUIT:
            return LOCALIZE("Tutorial");
        case GOAL_SCORE:
            return LOCALIZE("Tutorial");
    }
    return "";
}

uint8_t TableData::getLevelStars(uint16_t level_idx, uint32_t score)
{
    ValueMap& level_data = getLevelData(level_idx);
    ValueVector& level_score = level_data.at("Score").asValueVector();
    if (score < level_score.at(0).asInt()) return 0;
    else if (score < level_score.at(1).asInt()) return 1;
    else if (score < level_score.at(2).asInt()) return 2;
    else return 3;
}

bool TableData::init() {
    _levels_data = FileUtils::getInstance()->getValueMapFromFile("levels.plist");
    return true;
}

ValueMap& TableData::getLevelData(uint16_t level_idx)
{
    std::string level_name = "Level" + CCLanyard_Util::toString(level_idx);
    return getLevelData(level_name);
}

ValueMap& TableData::getLevelData(const std::string& level)
{
    return _levels_data.at(level).asValueMap();
}

TableGoal TableData::getLevelRule(uint16_t level_idx)
{
    std::string level_name = "Level" + CCLanyard_Util::toString(level_idx);
    return getLevelRule(level_name);

}
TableGoal TableData::getLevelRule(const std::string& level)
{
    ValueMap& level_data = getLevelData(level);
    std::string level_rules =   level_data.at("Rules").asString();
    
           if (level_rules.compare("destroy the under blocks") == 0) {
        return GOAL_UNDERBLOCKS;
    } else if (level_rules.compare("destroy gorgons eye") == 0) {
        return GOAL_DISPEL;
    } else if (level_rules.compare("destroy ice") == 0) {
        return GOAL_ICE;
    } else if (level_rules.compare("collect juice") == 0) {
        return GOAL_JUICE;
    } else if (level_rules.compare("collect fruit") == 0) {
        return GOAL_FRUIT;
    } else if (level_rules.compare("drop money") == 0) {
        return GOAL_COLLECT;
    } else if (level_rules.compare("release phenoix") == 0) {
        return GOAL_MATCH;
    } else if (level_rules.compare("score no fruit") == 0) {
        return GOAL_SCORE_NOFRUIT;
    } else if (level_rules.compare("score") == 0) {
        return GOAL_SCORE;
    } else {
        CCASSERT(false, "level rule not recognised");
    }
}
