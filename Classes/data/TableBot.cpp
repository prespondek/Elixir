//
//  TableAI.cpp
//  Elixir
//
//  Created by Peter Respondek on 6/14/17.
//
//

#include "../data/TableBot.h"
#include "../layers/TableLayer.h"
#include "../base/ccFPSImages.h"
#include "../layers/TableHUDLayer.h"
#include "../nodes/TableNodeSprite.h"



TableBot* TableBot::create( TableScene* scene )
{
    TableBot* pRet = new (std::nothrow) TableBot(scene);
    if (pRet && pRet->init()) {
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool TableBot::init()
{
    TableGraph* graph = _scene->getTable()->getGraph();
    std::map<TableNode*,Dijkstra<TableNode*>::GraphVertex*> verts;
    
    for ( auto node : graph->getNodes() ) {
        if ( node->getType() == TableNode::BLOCKED )
            continue;
        auto vert = _graph.createVertex();
        vert->data = node;
        verts.insert(std::pair<TableNode*,Dijkstra<TableNode*>::GraphVertex*>(node,vert));
    }
    
    for ( auto vert: verts) {
        std::vector<TableNode*> adj_nodes;
        TableNode* vert_node = vert.second->data;
        if ( vert_node->getType() == TableNode::PIPE ) {
            TableNodePipe* pipe = static_cast<TableNodePipe*> (vert_node);
            if (pipe->getEntryNode() == vert_node) {
                adj_nodes.push_back(pipe->getExitNode());
            } else {
                adj_nodes.push_back(pipe->getEntryNode());
            }
        }
        graph->getBorderNodes(vert.second->data, adj_nodes);
        for (auto node : adj_nodes ) {
            float dist = 0.0f;
            if ( vert_node->getType() == TableNode::PIPE  &&
                 node->getType() == TableNode::PIPE )
                dist = 1.0f;
            auto iter = verts.find(node);
            if ( iter != verts.end() ) {
                if (!dist)
                    dist = vert.second->data->getTablePosition().distance(iter->second->data->getTablePosition());
                vert.second->addEdge(iter->second,dist);
            }
        }
    }
    
    for ( uint8_t i = 0; i < TableScene::TABLEMODE_MAX; i++) {
        HUDButton* button = dynamic_cast<BoosterButton*>(_scene->getHUD()->getButton(TableScene::TableMode(i),false));
        if (button) _boosters.push_back(button);
    }
    return true;
}

TableHint* TableBot::getClosestHint ( uint8_t cutoff )
{
    std::vector<uint8_t> priority(_hints.size(),UINT8_MAX);
    
    uint16_t counter = 0;
    uint16_t min_idx = 0;
    float min_dist = MAXFLOAT;
    for ( auto hint: _hints ) {
        for ( auto goal : _goals ) {
            Lanyard::Dijkstra<TableNode*>::GraphVertex* vert1 = _graph.find(goal);
            float goal_dist = MAXFLOAT;
            for ( auto node : hint->match->nodes ) {
                Lanyard::Dijkstra<TableNode*>::GraphVertex* vert2 = _graph.find(node);
                goal_dist = MIN( goal_dist,_graph.calcPath( vert1, vert2, nullptr ) );
            }
            if ( goal_dist < min_dist ) {
                min_dist = goal_dist;
                min_idx = counter;
            }
        }
        counter++;
    }
    if (min_dist <= cutoff) {
        BOTLOG ("Found Closest Hint");
        _stats_lable->_hint_label->setString("TACTIC: Closest");
        return _hints.at(min_idx);
    }
    return nullptr;
}

TableHint* TableBot::getProximityHint ( )
{
    TableLayer* table = _scene->getTable();
    TableGraph* graph = table->getGraph();
    TableGoal goal = _scene->getRule();
    
    std::vector<TableNode*> bnodes;
    for ( auto mgoal : _goals ) {
        TableNode* target = mgoal->getDropNode( CC_CALLBACK_2(TableNode::isDropNode, mgoal),
                                                CC_CALLBACK_2(TableNode::isAdjacentDropNode, mgoal), mgoal->getOverBlock() );
        if ( !target ) {
            continue;
        }
        graph->getBorderNodes(mgoal, bnodes);
        for ( auto i = bnodes.begin(); i != bnodes.end(); ) {
            if (mgoal->isRestricted() != (*i)->isRestricted()) {
                bnodes.erase(i);
            } else {
                i++;
            }
        }
    }
    // get blocks we want to get proximity to.
    std::vector<TableNode*> gnodes;
    for ( auto node : bnodes ) {
        TableNode* target = node;
        while ( target && ( target->isContainer() || target->getType() == TableNode::PIPE ) ) {
            TableNode* old_target = target;
            if ( goal == GOAL_ICE ) {
                target = target->getPrevActiveNode();
            } else {
                target = target->getNextActiveNode();
                // if a coin is resting against the bottom not much point in a prox hint.
                if ( goal == GOAL_COLLECT && old_target == node && ( !target || target->getType() == TableNode::BLOCKED ) ) {
                    break;
                }
            }
            gnodes.push_back(old_target);
        }
    }
    std::sort(gnodes.begin(), gnodes.end());
    auto iter = std::unique(gnodes.begin(), gnodes.end());
    gnodes.resize(iter - gnodes.begin());
    
    std::vector<int> priortity;
    for ( auto hint : _hint_intersect ) {
        std::vector<TableNode*> inodes(MAX(gnodes.size(), hint.size()));
        auto iter = std::set_intersection(hint.begin(), hint.end(), gnodes.begin(), gnodes.end(), inodes.begin());
        inodes.resize(iter - inodes.begin());
        priortity.push_back(inodes.size());
    }
    int8_t idx = -1;
    for ( uint8_t i = 0; i < priortity.size(); i++ ) {
        if (priortity.at(i) > 0 && (idx < 0 || priortity.at(i) > priortity.at(idx) )) {
            idx = i;
        }
    }
    if (idx < 0) return nullptr;
    BOTLOG ("Found Proximity Hint");
    _stats_lable->_hint_label->setString("TACTIC: Proximity");
    return (_hints.at(idx));
}

void TableBot::getTargets( std::vector<TableNode*>& out_nodes, bool active )
{
    TableLayer* table = _scene->getTable();
    TableGoal goal = _scene->getRule();
    
    std::function<bool(TableBlock*)> block_func = nullptr;
    std::function<bool(TableNode*)> node_func = nullptr;
    
    switch (goal) {
        case GOAL_COLLECT:
            node_func = [active] (TableNode* node) { return node && (!active || node->isActive()) &&
                node->getType() == TableNode::COLLECTER; };
            break;
        default:
            break;
    }
    table->getGraph()->filterNodes(&out_nodes,node_func,block_func,nullptr);
    std::sort(out_nodes.begin(), out_nodes.end());
}

void TableBot::getGoals( std::vector<TableNode*>& out_nodes, bool active )
{
    TableLayer* table = _scene->getTable();
    TableGoal goal = _scene->getRule();
    BlockType type = NORMAL;
    
    std::function<bool(TableBlock*)> block_func = nullptr;
    std::function<bool(TableNode*)> node_func = nullptr;
    
    switch (goal) {
        case GOAL_DISPEL: type =    EYE; break;
        case GOAL_COLLECT: type =   COIN; break;
        case GOAL_MATCH: type =     EGG; break;
        case GOAL_JUICE: type =     FRUIT; break;
        case GOAL_ICE: type =       ICE; break;
        case GOAL_FRUIT: type =     GOAL; break;
        default:
            break;
    }

    switch (goal) {
        case GOAL_UNDERBLOCKS:
            node_func = [active] (TableNode* node)    { return node  && (!active || node->isActive()) && node->getUnderBlock(); };
            break;
        case GOAL_DISPEL:
        case GOAL_COLLECT:
        case GOAL_MATCH:
        case GOAL_JUICE:
        case GOAL_FRUIT:
        case GOAL_ICE:
            block_func = [active,type] (TableBlock* block) -> bool {
                if (!block) return false;
                TableBlock* prisoner = nullptr;
                TableBlockPrison* prison = dynamic_cast<TableBlockPrison*>(block);
                if (prison) prisoner = prison->getPrisoner();
                return (!active || block->getNode()->isActive()) &&
                (block->getType() == type || (prisoner && prisoner->getType() == type)); };
            break;
        case GOAL_SCORE_NOFRUIT: break;
        case GOAL_SCORE: break;
        default:
            break;
    }
    table->getGraph()->filterNodes(&out_nodes,node_func,block_func,nullptr);
    std::sort(out_nodes.begin(), out_nodes.end());

}

TableHint* TableBot::getBlockedHint ( )
{
    TableGoal goal = _scene->getRule();
    
    auto func = [] (TableNode* target) ->bool {
        return (target && target->getOverBlock() &&
         (target->getOverBlock()->getType() == BLOCKER ||
          target->getOverBlock()->getType() == JELLY ||
          target->getOverBlock()->isPetrified() ||
          target->getOverBlock()->getType() == ICE));
    };
    
    std::set<TableNode*> gnodes;
    if ( goal == GOAL_COLLECT )  {
        for ( auto i : _goals) {
            TableNode* target = i;
            
            while (target && ( target->isContainer() || target->getType() == TableNode::PIPE ) ) {
                if (func(target))
                {
                    gnodes.insert(target);
                }
                target = target->getNextActiveNode();
            }
        }
    } else {
        for ( auto goal : _goals ) {
            Lanyard::Dijkstra<TableNode*>::GraphVertex* vert1 = _graph.find(goal);
            for ( auto hint : _hints ) {
                TableNode* node = hint->match->nodes.at(0);
                Lanyard::Dijkstra<TableNode*>::GraphVertex* vert2 = _graph.find(node);
                std::vector<TableNode*> path;
                _graph.calcPath( vert1, vert2, &path );
                for ( auto p : path ) {
                    if (func(p))
                        gnodes.insert(p);
                }
            }
        }
    }
    for ( auto i : gnodes ) {
        for ( uint8_t j = 0; j < 8; j +=2 ) {
            TableNode* adj_node = i->getAdjacentNode(TableNode::TableDir(j),func);
            if ( adj_node ) gnodes.insert(adj_node);
        }
    }
    std::set<TableNode*> dnodes(gnodes);
    for ( auto i : gnodes ) {
        for ( uint8_t j = 0; j < 8; j +=2 ) {
            TableNode* adj_node = i->getAdjacentNode(TableNode::TableDir(j),[func] (TableNode* target) {
                return !func(target) && ( target->isContainer() || target->getType() == TableNode::PIPE );
            });
            if ( adj_node ) dnodes.insert(adj_node);
        }
    }
    std::vector<TableNode*> tnodes(MAX(dnodes.size(), gnodes.size()));
    {
    auto iter = std::set_symmetric_difference(dnodes.begin(), dnodes.end(),
                                              gnodes.begin(), gnodes.end(),
                                              tnodes.begin());
    tnodes.resize(iter - tnodes.begin());
    }
    std::sort(tnodes.begin(), tnodes.end());
    std::vector<int> priortity;
    for ( uint8_t i = 0; i < _hints.size(); i++ ) {
        auto hint = _hints.at(i);
        auto hinti = _hint_intersect.at(i);
        if ( hint->match->match_type == TableMatch::MATCH_TWO_MIRACLE ) {
            priortity.push_back(INT32_MAX);
            continue;
        }
        std::vector<TableNode*> inodes(MAX(1,MAX(gnodes.size(),MAX(tnodes.size(), hinti.size()))));
        auto hnodes(hint->match->nodes);
        std::sort(hnodes.begin(), hnodes.end());
        auto iter = std::set_intersection(hnodes.begin(), hnodes.end(),
                                          tnodes.begin(), tnodes.end(),
                                          inodes.begin());
        iter = std::set_intersection(hinti.begin(), hinti.end(),
                                     gnodes.begin(), gnodes.end(),
                                     iter);
        std::sort(inodes.begin(), inodes.end());
        iter = std::unique(inodes.begin(), inodes.end());
        inodes.resize(std::distance(inodes.begin(),iter));
        iter = std::remove(inodes.begin(), inodes.end(),nullptr);
        inodes.resize(std::distance(inodes.begin(),iter));
        int score = INT32_MAX;
        if (inodes.size()) {
            score = inodes.size();
            uint8_t min = UINT8_MAX;
            for ( auto h : inodes ) {
                for ( auto goal : _goals ) {
                    min = MIN(min,h->getTablePosition().distance(goal->getTablePosition()));
                }
            }
            score -= min;
        }
        priortity.push_back(score);
    }
    int8_t idx = -1;
    for ( uint8_t i = 0; i < priortity.size(); i++ ) {
        if (priortity.at(i) != INT32_MAX && (idx < 0 || priortity.at(i) > priortity.at(idx) )) {
            idx = i;
        }
    }
    if (idx < 0) return nullptr;
    BOTLOG ("Found Blocked Hint");
    _stats_lable->_hint_label->setString("TACTIC: Blocked");
    return (_hints.at(idx));
}

TableHint* TableBot::getMatchHint ()
{
    TableHint* out_hint = nullptr;
    
    for (auto hint : _hints) {
        for ( auto gblock : _goals ) {
            if ( hint->match->in(gblock) || hint->source == gblock )
                out_hint = hint;
        }
    }
    if (out_hint) {
        BOTLOG ("Found Match Hint");
        _stats_lable->_hint_label->setString("TACTIC: Match");
        return out_hint;
    }
    return nullptr;
}

TableHint* TableBot::getFruitHint ()
{
    TableGoal goal = _scene->getRule();

    if (goal != GOAL_JUICE && _scene->getHUD()->getManaBarFG()->getPercentage() >= 1.0f )
        return nullptr;
        
    TableLayer* table = _scene->getTable();
    std::vector<int> priority;
    for (auto hint : _hints) {
        uint8_t counter = 0;
        for ( auto block : hint->match->blocks ) {
            if (block->getType() == FRUIT)
                counter++;
        }
        if ( hint->source->getOverBlock()->getType() == FRUIT )
            counter++;
        
        if ( counter ) {
            for ( auto block : hint->match->blocks ) {
                if (block->getType() == OMNI ||
                    block->getType() == MIRACLE)
                    counter++;
            }
        }
        priority.push_back(counter);
    }
     
    for ( uint8_t i = 0; i < _hints.size(); i++ ) {
        for ( auto node : _hints.at(i)->match->nodes ) {
            TableNode* target = node;
            while (target && ( target->isContainer() || target->getType() == TableNode::PIPE ) ) {
                target = target->getPrevActiveNode();
                if ( target && target->getOverBlock() && target->getOverBlock()->getType() == EYE ) {
                    priority.at(i) = 0;
                }
            }
        }
    }
    
    
    int8_t idx = -1;
    for ( uint8_t i = 0; i < priority.size(); i++ ) {
        if (priority.at(i) > 0 && (idx < 0 || priority.at(i) > priority.at(idx) )) {
            idx = i;
        }
    }
    if (idx < 0) return nullptr;
    BOTLOG ("Found Fruit Hint");
    _stats_lable->_hint_label->setString("TACTIC: Fruit");
    return (_hints.at(idx));
}



TableHint* TableBot::getMatchFives    ( )
{
    for ( auto hint : _hints ) {
        if ( hint->match->match_type == TableMatch::MATCH_FIVE_LINE ||
             hint->match->match_type == TableMatch::MATCH_SEVEN ||
            hint->match->match_type == TableMatch::MATCH_TWO_OMNI ||
             ( hint->match->match_type == TableMatch::MATCH_OMNI && hint->match->type != NORMAL && hint->match->type != FRUIT))
        {
            _stats_lable->_hint_label->setString("TACTIC: Match 5");
            return hint;
        }
    }
    return nullptr;
}

TableHint* TableBot::getPredictiveHint ( uint8_t min )
{
    TableLayer* table = _scene->getTable();
    TableGoal goal = _scene->getRule();
    TableGraph* graph = table->getGraph();
    
    std::vector<int> priortity (_hints.size(), 0);
    for ( uint16_t i = 0; i < _hints.size(); i++ ) {
        auto hint = _hints.at(i);
        graph->swapBlocks(hint->source, hint->match->nodes.at(0));

        std::vector<TableNode*> prev_nodes;
        for ( auto node : hint->match->nodes ) {
            TableNode* prev = node->getPrevActiveNode();
            auto iter = std::find(hint->match->nodes.begin(), hint->match->nodes.end(), prev);
            if ( prev == nullptr || iter != hint->match->nodes.end() ||
                !prev->getOverBlock() || !prev->getOverBlock()->willMoveBlock())
                continue;
            prev_nodes.push_back(prev);
        }
        
        std::vector<TableNode*> next_nodes;
        for ( auto node : prev_nodes ) {
            TableNode* next = node;
            while(1)  {
                TableNode* new_next = next->getNextActiveNode();
                if ( new_next == nullptr || std::find(hint->match->nodes.begin(),
                                               hint->match->nodes.end(),
                                               new_next) == hint->match->nodes.end())
                    break;
                next = new_next;
            }
            next_nodes.push_back(next);
        }
        
        for ( uint8_t j = 0; j < prev_nodes.size(); j++ ) {
            graph->swapBlocks(next_nodes.at(j), prev_nodes.at(j));
        }
        
        for ( auto node : next_nodes ) {
            std::vector<TableHint*> hints;
            graph->checkForDirectionHint(node, hints);
            for ( auto& hint : hints ) {
                std::vector<TableNode*> hint_nodes;
                getMatchNodes(hint_nodes, hint->match);
                std::vector<TableNode*> inodes (_goals.size(),nullptr);
                auto iter = std::set_intersection(hint_nodes.begin(), hint_nodes.end(),
                                                  _goals.begin(), _goals.end(),
                                                  inodes.begin());
                inodes.resize(iter - inodes.begin());
                priortity.at(i) += inodes.size();
            }
        }
        
        for ( uint8_t j = 0; j < prev_nodes.size(); j++ ) {
            graph->swapBlocks(next_nodes.at(j), prev_nodes.at(j));
        }
        
        graph->swapBlocks(hint->source, hint->match->nodes.at(0));
        
        auto type = table->getInsertBlockType(hint->match);
        if ( type != NORMAL && type != MISSLE ) {
            std::vector<TableNode*> nodes;
            table->getDestructNodes(nodes, hint->match->nodes.at(0), table->getDestructPattern(type),
                                    BlockColor::NONE, BlockType::NORMAL, hint->match->nodes.at(0)->isRestricted());
            std::sort(nodes.begin(), nodes.end());
            std::vector<TableNode*> inodes(MAX(nodes.size(),_goals.size()));
            auto iter = std::set_intersection(nodes.begin(), nodes.end(),
                                              _goals.begin(), _goals.end(),
                                              inodes.begin());
            inodes.resize(iter - inodes.begin());
            priortity.at(i) += inodes.size();
        }
    }
    int8_t idx = -1;
    for ( uint8_t i = 0; i < priortity.size(); i++ ) {
        if (priortity.at(i) > min && (idx < 0 || priortity.at(i) >= priortity.at(idx) )) {
            idx = i;
        }
    }
    if (idx < 0) return nullptr;
    BOTLOG ("Found Predictive Hint");
    _stats_lable->_hint_label->setString("TACTIC: Predictive");
    return (_hints.at(idx));
}

TableHint* TableBot::getDestructHint ()
{
    TableGoal goal = _scene->getRule();
    auto goals = _goals;
    uint8_t min = 0;
    switch (goal) {
        case GOAL_COLLECT:
            for ( uint8_t i = 0; i < goals.size(); ) {
                if ( goals.at(i)->getOverBlock()->getFallState() != STATIC )
                    goals.erase(goals.begin()+i);
                else
                    i++;
            }
            break;
        case GOAL_JUICE:
            min = 3;
        default:
            break;
    }

    if (goals.empty())
        return nullptr;
    
    std::vector<float> priortity;
    for ( uint8_t i = 0; i < _hints.size(); i++ ) {
        auto hinti = _hint_intersect.at(i);
        float sub = 0;
        for (auto node : hinti) {
            if ( goal == GOAL_JUICE &&
                 node->getOverBlock() &&
                 node->getOverBlock()->getType() == MIRACLE ) {
                sub++;
            }
        }
        std::vector<TableNode*> inodes(MAX(hinti.size(), goals.size()));
        auto iter = std::set_intersection(hinti.begin(), hinti.end(),
                                          goals.begin(), goals.end(),
                                          inodes.begin());
        inodes.resize(iter - inodes.begin());
        float match_pir = 0;
        if (inodes.size() && _hints.at(i)->match->match_type > TableMatch::MATCH_THREE)
            match_pir += 0.5f;
        priortity.push_back(int(inodes.size()) - sub + match_pir);
        
            
    }
    int8_t idx = -1;
    for ( uint8_t i = 0; i < priortity.size(); i++ ) {
        if (priortity.at(i) > min && (idx < 0 || priortity.at(i) >= priortity.at(idx) )) {
            idx = i;
        }
    }
    if (idx < 0) return nullptr;
    BOTLOG ("Found Destruct Hint");
    _stats_lable->_hint_label->setString("TACTIC: Destruct");
    return (_hints.at(idx));
}

void TableBot::getGoalDropNodes (std::vector<TableNode*>& goals, std::vector<std::vector<TableNode*>>& gnodes)
{
    for ( auto i : goals) {
        if ( i->getOverBlock() && i->getOverBlock()->getFallState() == STATIC ) continue;
        TableNode* target = i->getDropNode( CC_CALLBACK_2(TableNode::isDropNode, i),
                                            CC_CALLBACK_2(TableNode::isAdjacentDropNode, i), i->getOverBlock() );
        std::vector<TableNode*> nodes;
        while (target && ( target->getType() == TableNode::PIPE  ||
                          ( target->getOverBlock() && !target->getOverBlock()->isInvinsible() ))) {
            nodes.push_back(target);
            target = target->getDropNode(CC_CALLBACK_2(TableNode::isDropNode, target) ,
                                         CC_CALLBACK_2(TableNode::isAdjacentDropNode, target) , i->getOverBlock());
        }
        gnodes.push_back(nodes);
    }
}

void TableBot::getGoalDropNodes (std::vector<TableNode*>& goals, std::vector<TableNode*>& gnodes)
{
    std::vector<std::vector<TableNode*>> dnodes;
    getGoalDropNodes(goals, dnodes);
    for ( auto& nodes : dnodes ) {
        gnodes.insert(gnodes.end(), nodes.begin(),nodes.end());
    }
    std::sort(gnodes.begin(), gnodes.end());
}

void TableBot::getTargetDropNodes (std::vector<TableNode*>& goals, std::vector<std::vector<TableNode*>>& gnodes, bool direct)
{
    auto func = std::bind(&TableNode::getPrevActiveNode, std::placeholders::_1);
    if (direct)
        func = std::bind(&TableNode::getPointActiveNode, std::placeholders::_1);
    for ( auto i : goals) {
        TableNode* target = func(i);
        std::vector<TableNode*> nodes;
        while (target && (target->isContainer() || target->getType() == TableNode::PIPE)) {
            nodes.push_back(target);
            target = func(target);
        }
        gnodes.push_back(nodes);
    }
}

void TableBot::getTargetDropNodes (std::vector<TableNode*>& goals, std::vector<TableNode*>& gnodes)
{
    std::vector<std::vector<TableNode*>> dnodes;
    getTargetDropNodes(goals, dnodes);
    for ( auto& nodes : dnodes ) {
        gnodes.insert(gnodes.end(), nodes.begin(),nodes.end());
    }
    std::sort(gnodes.begin(), gnodes.end());
}


bool TableBot::getDispelHint ( TableBlock** block )
{
    TableLayer* table = _scene->getTable();
    TableGoal goal = _scene->getRule();
    std::vector<std::vector<TableNode*>> gnodes;
    getGoalDropNodes(_goals, gnodes);
    
    uint8_t max_dist = UINT8_MAX;
    switch (goal) {
        case GOAL_DISPEL:
            for ( uint8_t i = 0; i < _goals.size(); i++ ) {
                TableNode* goal = _goals.at(i);
                uint8_t dist = gnodes.at(i).size();
                if ( goal->isActive() && goal->getOverBlock()->getType() == EYE && dist < max_dist ) {
                    max_dist = dist;
                    booster_hint_1 = goal;
                    if (block)
                        *block = goal->getOverBlock();
                }
            }
            break;
        default:
            break;
            
    }
    if (booster_hint_1) return true;
    return false;
}

bool TableBot::getHammerHint ( TableBlock** block, uint8_t opt )
{
    TableLayer* table = _scene->getTable();
    TableGoal goal = _scene->getRule();
    TableGraph* graph = table->getGraph();
    
    std::vector<std::vector<TableNode*>> gnodes;
    std::vector<TableNode*> nodes;
    int8_t min = 0;
    
    table->getGraph()->filterNodes( &nodes,
                                   [] (TableNode* node) { return node->isActive() && node->isContainer(); },
                                   [] (TableBlock* block) { return block && !block->isInvinsible(); } , nullptr);
    
    std::vector<int> priortity(nodes.size(),INT32_MIN);
    
    switch (goal) {
        case GOAL_COLLECT:
            getGoalDropNodes(_goals, gnodes);
            if (opt > 0)
                min = -2;
            else
                min = INT8_MIN;
            break;
        case GOAL_ICE:
            if (_goals.size() > 3)
                min = 2;
            gnodes.push_back(_goals);
            break;
        case GOAL_JUICE:
            min = 3;
            gnodes.push_back(_goals);
            break;
        default:
            gnodes.push_back(_goals);
            break;
    }
    for (auto& gnode : gnodes)
        std::sort(gnode.begin(), gnode.end());
    
    for ( uint8_t i = 0; i < nodes.size(); i++ ) {
        TableNode* node = nodes.at(i);
        std::vector<TableNode*> dnodes;
        TableBlock* block = node->getOverBlock();
        if (block) {
            TableBlockPrison* prisoner = dynamic_cast<TableBlockPrison*>(block);
            if (prisoner)
                block = prisoner->getPrisoner();
            table->getDestructNodes(dnodes, node, table->getDestructPattern(block),BlockColor::NONE, BlockType::NORMAL, node->isRestricted());
        }
        std::sort(dnodes.begin(), dnodes.end());
        for (auto& gnode : gnodes) {
            std::vector<TableNode*> inodes;
            inodes.resize(MAX(dnodes.size(), gnodes.size()));
            auto iter = std::set_intersection( dnodes.begin(), dnodes.end(),
                                               gnode.begin(), gnode.end(),
                                               inodes.begin() );
            inodes.resize(iter - inodes.begin());
            if ( inodes.size() ) {
                if ( priortity.at(i) == INT32_MIN )
                    priortity.at(i) = 0;
                priortity.at(i) += inodes.size();
                if (goal == GOAL_COLLECT && inodes.size() ) {
                    if ( block && block->getType() == BLOCKER )
                        priortity.at(i) += 32;
                    priortity.at(i) -= gnode.size();
                }
            }
        }
    }
    
    int8_t idx = -1;
    for ( uint8_t i = 0; i < priortity.size(); i++ ) {
        if ( priortity.at(i) != INT_MIN && (idx < 0 || priortity.at(i) > priortity.at(idx) )) {
            idx = i;
        }
    }
    
    if ( idx != -1 && priortity.at(idx) >= min) {
        booster_hint_1 = nodes.at(idx);
        if (block)
            *block = nodes.at(idx)->getOverBlock();
        return true;
    }
    return false;
}

bool TableBot::getTeleportHint ( TableBlock** block )
{
    TableLayer* table = _scene->getTable();
    TableGoal goal = _scene->getRule();
    TableGraph* graph = table->getGraph();
    
    // filter out any goals that are already in good positions.
    std::vector<TableNode*> goals = _goals;
    for ( auto i = goals.begin(); i != goals.end();) {
        std::vector<TableNode*> nodes;
        graph->getIsland(nodes, *i, [] (TableNode* nnode) -> bool {
            return (nnode->getType() != TableNode::BLOCKED);
        });
        bool island = true;
        if (goal != GOAL_UNDERBLOCKS && goal != GOAL_COLLECT ) {
            for ( auto node : nodes ) {
                if ( node->isSpawner() || node->getType() == TableNode::SPAWN )
                    island = false;
            }
        }
        if ( (*i)->getType() == TableNode::NO_TELEPORT || !island || !graph->canTeleportBlock((*i)->getOverBlock()) )
            goals.erase(i);
        else
            i++;
    }
    if ( goals.size() == 0 && goal != GOAL_UNDERBLOCKS ) return false;

    std::vector<TableNode*> targets;
    
    std::vector<int> priortity;
    uint16_t min = 0;

    switch (goal) {
        case GOAL_UNDERBLOCKS:
            min = 1;
            if (goals.size() > 10) min = 4;
        case GOAL_MATCH: {
            struct mtype {
                BlockColor color;
                std::set<BlockType> types;
                mtype (BlockColor color, BlockType type) {
                    this->color = color;
                    types.insert(type);
                }
                bool operator==(const mtype& rhs) const
                { return color == rhs.color; }
                bool operator<(const mtype& rhs) const
                { return color < rhs.color; }
            };
            BlockColor best_color;
            BlockType best_type;
            std::vector<TableNode*> best_nodes;
            uint32_t target_priority = 0;
            std::vector<mtype> colors;
            graph->filterNodes(&targets, [] ( TableNode* node ) { return node && node->isActive(); },
                               [ graph ] ( TableBlock* block ) -> bool { if (graph->canTeleportBlock(block)) {
                std::vector<TableNode*> inodes;
                block->getNode()->getAdjacentNodes(inodes, [ block ] (TableNode* node) -> bool {
                    return node && node->isActive() && node->getOverBlock() && node->getOverBlock()->isObjective() && node->getOverBlock()->getColor() == block->getColor();
                });
                if (inodes.size())
                    return false;
                return true;
            } return false; },
               [ &colors ] ( TableNode* node ) {
                   BlockColor color = node->getOverBlock()->getColor();
                   BlockType type = node->getOverBlock()->getType();
                   auto iter = std::find_if(colors.begin(), colors.end(), [color] (mtype type) -> bool { return type.color == color; } );
                   if (iter != colors.end()) {
                       iter->types.insert(type);
                   }
                   else colors.push_back(mtype(color, type));
               });
            for ( auto node : targets ) {
                for ( auto mcolor : colors ) {
                    if (node->getOverBlock()->getColor() == mcolor.color)
                        continue;
                    BlockColor color = mcolor.color;
                    BlockColor prev_color = node->getOverBlock()->getColor();
                    node->getOverBlock()->setBlockColor(mcolor.color);
                    BlockType prev_type = node->getOverBlock()->getType();
                    for ( auto type : mcolor.types ) {
                        node->getOverBlock()->setBlockType(type);
                        TableMatch* match = graph->checkForMatchs(node, false);
                        std::vector<TableNode*> inodes;
                        uint16_t score = 0;
                        if (match) {
                            inodes.resize(_goals.size());
                            std::vector<TableNode*> bnodes;
                            if (match->target_color == mcolor.color) {
                                getMatchNodes(bnodes, match);
                            }
                            if ( match->match_type == TableMatch::MATCH_FIVE_LINE ||
                                match->match_type == TableMatch::MATCH_SEVEN )
                                score = 5;
                            auto iter = std::set_intersection(_goals.begin(), _goals.end(),
                                                              bnodes.begin(), bnodes.end(),
                                                              inodes.begin());
                            inodes.resize(iter - inodes.begin());
                            /*for ( auto i = inodes.begin(); i != inodes.end();) {
                                if ( (*i)->isRestricted() && !node->isRestricted() ) {
                                    inodes.erase(i);
                                } else {
                                    i++;
                                }
                            }*/
                        } else if ( goal == GOAL_MATCH ) {
                            node->getAdjacentNodes(inodes, [ color ] (TableNode* node) -> bool {
                                return (node && node->isActive() && node->getOverBlock() &&
                                        node->getOverBlock()->getColor() == color &&
                                        node->getOverBlock()->isObjective());
                            });
                        }
                        score = MAX(inodes.size(),score);
                        if ( target_priority < score && score > min ) {
                            target_priority = score;
                            booster_hint_1 = node;
                            best_color = color;
                            best_type = type;
                            best_nodes = match->nodes;
                        }
                        if (match)
                            delete match;
                    }
                    node->getOverBlock()->setBlockType(prev_type);
                    node->getOverBlock()->setBlockColor(prev_color);
                }
            }
            if ( target_priority ) {
                std::vector<TableNode*> new_target;
                std::sort(best_nodes.begin(),best_nodes.end());
                std::sort(targets.begin(), targets.end());
                new_target.resize(MAX(best_nodes.size(),targets.size()));
                auto iter = std::set_difference(targets.begin(), targets.end(),
                                                best_nodes.begin(), best_nodes.end(),
                                                new_target.begin());
                new_target.resize(iter - new_target.begin());
                std::random_shuffle(new_target.begin(), new_target.end());

                for ( auto node : new_target ) {
                    if (node->getOverBlock()->getColor() == best_color && node->getOverBlock()->getType() == best_type && booster_hint_2 != booster_hint_1) {
                        booster_hint_2 = node;
                        break;
                    }
                }
            }
        }
            break;
        case GOAL_FRUIT: {
            uint8_t score = 0;
            graph->filterNodes(&targets, nullptr,
               [ graph ] ( TableBlock* block ) {
                   return ( graph->canTeleportBlock(block) && !block->getNode()->isRestricted() ) && block->getType() != GOAL; },
               [ this, &priortity, &goals, graph, score ] ( TableNode* block ) mutable {
                   for ( auto goal : goals ) {
                       graph->swapBlocks(block, goal);
                       TableMatch* match1 = graph->checkForMatchs(block, false);
                       TableMatch* match2 = graph->checkForMatchs(goal, false);
                       if (match1 || match2) {
                           std::vector<TableNode*> bnodes;
                           std::vector<TableNode*> inodes;
                           getMatchNodes(bnodes, match1);
                           getMatchNodes(bnodes, match2);
                           std::sort(bnodes.begin(), bnodes.end());
                           auto iter = std::unique(bnodes.begin(), bnodes.end());
                           bnodes.resize(std::distance(bnodes.begin(),iter));
                           inodes.resize(MIN(_goals.size(), bnodes.size()));
                           iter = std::set_intersection(_goals.begin(), _goals.end(),
                                                        bnodes.begin(), bnodes.end(),
                                                        inodes.begin());
                           inodes.resize(iter - inodes.begin());
                           if (match1) delete match1;
                           if (match2) delete match2;
                           if (inodes.size() > score) {
                               score = inodes.size();
                               booster_hint_1 = goal;
                               booster_hint_2 = block;
                           }
                       }
                       graph->swapBlocks(block, goal);
                   }
                   priortity.push_back(0);
               } );
        }
            break;
        case GOAL_COLLECT: {
            getTargets(targets);
            std::vector<std::vector<TableNode*>> tnodes;
            getTargetDropNodes(targets, tnodes, true);
            if (tnodes.size() == 0) return false;
            targets.clear();

            for (uint8_t i = 0; i < tnodes.size(); i++) {
                auto& nodes = tnodes.at(i);
                bool block = false;
                for (uint8_t j = 0; j < nodes.size(); j++) {
                    if ( ( !nodes.at(j)->getOverBlock() ||
                         ( graph->canTeleportBlock(nodes.at(j)->getOverBlock() ) &&
                           !nodes.at(j)->getOverBlock()->isObjective()) ) ) {
                        priortity.push_back(-j);
                        targets.push_back(nodes.at(j));
                        if (block == false) {
                            auto iter = std::find(goals.begin(), goals.end(), nodes.at(j));
                            if (iter != goals.end()) {
                                goals.erase(iter);
                            }
                        }
                        block = true;
                        break;
                    }
                }
                if (!block) {
                    priortity.push_back(INT_MIN);
                    targets.push_back(nullptr);
                }
            }
        }
            break;
        default:
            break;
    }

    int8_t idx = -1;
    for ( uint8_t i = 0; i < priortity.size(); i++ ) {
        if ( priortity.at(i) != INT_MIN && (idx < 0 || priortity.at(i) > priortity.at(idx) )) {
            idx = priortity.at(i);
        }
    }
    
    std::vector<TableNode*> new_targets;
    for ( uint8_t i = 0; i < priortity.size(); i++ ) {
        if (priortity.at(i) == idx) {
            new_targets.push_back(targets.at(i));
        }
    }
    
    if ( idx != -1 && !booster_hint_1 && !booster_hint_2 ) {
        booster_hint_1 = goals.at( rand() % goals.size() );
        booster_hint_2 = new_targets.at( rand() % new_targets.size() );
    }
    
    if (booster_hint_2 && booster_hint_1) {
        if (block)
            *block = booster_hint_1->getOverBlock();
        return true;
    }
    return false;
}

void TableBot::getBlockedNodes ( std::vector<TableNode*>& out_nodes )
{
    TableLayer* table = _scene->getTable();
    TableGraph* graph = table->getGraph();
    
    auto node_func = [graph] (TableNode* node) -> bool { return (node && node->isActive() && node->isTile()); };
    auto block_func = [graph] (TableBlock* block) -> bool { return (block && (block->getFallState() == STATIC || block->willBlockShake()) && !block->isInvinsible()); };
    
    graph->filterNodes(&out_nodes,node_func,block_func,nullptr);
}

void TableBot::getShakeNodes ( std::vector<TableNode*>& out_nodes )
{
    TableLayer* table = _scene->getTable();
    TableGraph* graph = table->getGraph();

    auto node_func = [graph] (TableNode* node) -> bool { return (node && node->isActive() && node->isTile()); };
    auto block_func = [graph] (TableBlock* block) -> bool { return (block && block->willBlockShake()); };
    
    std::vector<TableNode*> shake_nodes;
    graph->filterNodes(&shake_nodes,node_func,block_func,nullptr);
    
    std::vector<TableNode*> blocked_nodes;
    graph->getBoundryNodes( out_nodes, shake_nodes, [&node_func, graph] (TableNode* node) -> bool {
                return node && node_func(node) && graph->canSwapBlock(node->getOverBlock()); });
}

bool TableBot::getPaintHint ( TableBlock** block, TableHint** hint )
{
    TableLayer* table = _scene->getTable();
    TableGoal goal = _scene->getRule();
    TableGraph* graph = table->getGraph();
    
    BlockColor color = graph->getFruitColor();
    
    float min = 2.0f;
    
    std::vector<std::vector<TableNode*>> gnodes;

    auto node_func = [graph] (TableNode* node) -> bool { return (node && node->isActive() && node->getOverBlock() && graph->canSwapBlock(node->getOverBlock())); };
    auto block_func = [graph] (TableBlock* block) -> bool { return (block && graph->canPaintBlock(block)); };
    std::vector<TableNode*> nodes;
    table->getGraph()->filterNodes(&nodes,node_func,block_func,nullptr);
    
    switch (goal) {
        case GOAL_COLLECT: {
            getGoalDropNodes(_goals, gnodes);
        }
            break;
        case GOAL_UNDERBLOCKS:
            min = 0.0f;
        case GOAL_MATCH:
        case GOAL_FRUIT:
        case GOAL_JUICE:
        case GOAL_ICE:
        {
            gnodes.push_back(_goals);
        }
            break;
        default:
            break;
    }

    for (auto& gnode : gnodes ) {
        std::set<TableNode*> adj_nodes;
        for (auto& node : gnode ) {
            std::vector<TableNode*> island_nodes;
            graph->getIsland(island_nodes, node, [] (TableNode* node) -> bool {
                return node->isActive() && node->getOverBlock() && (node->getOverBlock()->getType() == ICE || 
                                                node->getOverBlock()->getType() == BLOCKER);
            });
            std::vector<TableNode*> blocked_nodes;
            graph->getBoundryNodes( blocked_nodes, island_nodes, [&node_func, &block_func] (TableNode* node) -> bool {
                return node && node_func(node) && block_func(node->getOverBlock());
            });
            adj_nodes.insert(blocked_nodes.begin(), blocked_nodes.end());
        }
        std::copy(adj_nodes.begin(),adj_nodes.end(), std::back_inserter(gnode));
        std::sort(gnode.begin(), gnode.end());
    }
    
    // we iterate through nodes are change the color of blocks
    uint16_t best_el [3] = {0,0,0};
    TableNode* best_node = nullptr;
    bool blocked = false;
    for ( auto node : nodes ) {
        BlockColor prev_color = node->getOverBlock()->getColor();
        node->getOverBlock()->setBlockColor(color);
        TableMatch* match = graph->checkForMatchs(node, false);
        std::vector<TableNode*> bnodes;
        bool qual = false;
        if (match) {
            if (match->target_color == color) {
                if (match->match_type == TableMatch::MATCH_SEVEN ||
                    match->match_type == TableMatch::MATCH_FIVE_LINE )
                    qual = true;
                getMatchNodes(bnodes, match);
            }
            delete match;
        } else {
            for ( uint8_t i = 0; i <= 6; i+= 2  ) {
                TableNode* adj = node->getAdjacentNode(TableNode::TableDir(i), node_func);
                if (adj && adj->getOverBlock() && adj->getOverBlock()->getColor() == color) {
                    bnodes.push_back(adj);
                }
            }
            if (bnodes.size()) {
                bnodes.push_back(node);
                std::sort(bnodes.begin(),bnodes.end());
            }

        }
        for (auto& gnode : gnodes) {
            std::vector<TableNode*> inodes(nodes.size());
            auto iter = std::set_intersection(gnode.begin(), gnode.end(),
                                              bnodes.begin(), bnodes.end(),
                                              inodes.begin());
            
            inodes.resize(iter - inodes.begin());
            if ( inodes.size() > best_el[0] || (best_el[0] == 0 && qual)) {
                best_el[0] = inodes.size();
                if ( inodes.size() == 0 ) {
                    best_el[0] = 1;
                }
                best_el[1] = gnode.size();
                best_el[2] = bnodes.size();
                best_node = node;
            }
        }
        node->getOverBlock()->setBlockColor(prev_color);
    }
    if ( best_el[0] > 0 && ( best_el[0] > min || best_el[1] < 3 ) ) {
        booster_hint_1 = best_node;
        //*block = best_node->getOverBlock();
        if ( best_el[2] > 2 )
            return true;
    }
    switch (goal) {
        case GOAL_JUICE: {
            return false;
        }
            default:
            break;
    }
    std::set<BlockColor> bc;
    for ( auto node : nodes ) {
        for ( uint8_t o_color = 0; o_color < num_block_colors; o_color++ ) {
            BlockColor color = (BlockColor)o_color;
            BlockColor prev_color = node->getOverBlock()->getColor();
            node->getOverBlock()->setBlockColor(color);
            TableMatch* match = graph->checkForMatchs(node, false);
            if (match) {
                std::vector<TableNode*> bnodes;
                getMatchNodes(bnodes, match);
                for (auto& gnode : gnodes) {
                    std::vector<TableNode*> inodes(nodes.size());
                    auto iter = std::set_intersection(gnode.begin(), gnode.end(),
                                                      bnodes.begin(), bnodes.end(), inodes.begin());
                    inodes.resize(iter - inodes.begin());
                    if ( inodes.size() ) {
                        if (bnodes.size() > 2)
                            blocked = true;
                        bc.insert(color);
                        booster_hint_1 = nullptr;
                    }
                }
                delete match;
            }
            node->getOverBlock()->setBlockColor(prev_color);
        }
    }
    
    for ( auto h: _hints) {
        auto iter = bc.find(h->match->target_color);
        if (iter != bc.end() && !h->match->isMatchTwo()) {
            booster_hint_1 = nullptr;
            *hint = h;
            return false;
        }
    }
    if ( blocked )
        booster_hint_1 = nullptr;
    if (booster_hint_1)
        return true;
    return false;
}

bool TableBot::getSwapHint ( TableBlock** block )
{
    TableLayer* table = _scene->getTable();
    TableGoal goal = _scene->getRule();
    TableGraph* graph = table->getGraph();
    std::vector<std::vector<TableNode*>> gnodes;
    
    auto func = [graph] (TableNode* node) -> bool { return (node && node->isActive() && node->getOverBlock() && graph->canSwapBlock(node->getOverBlock())); };
    std::vector<TableNode*> nodes;
    table->getGraph()->filterNodes(&nodes,func,nullptr,nullptr);

    bool check_targets = false;
    
    switch (goal) {
        case GOAL_COLLECT: {
            check_targets = true;
            getGoalDropNodes(_goals, gnodes);
            for (uint8_t i = 0; i < gnodes.size(); i++) {
                auto& nodes = gnodes.at(i);
                if (nodes.size() < 3 && nodes.size() > 0 &&
                    nodes.back()->getNextNode()->getType() == TableNodeBlocked::COLLECTER ) {
                    booster_hint_1 = _goals.at(i);
                    booster_hint_2 = _goals.at(i)->getNextActiveNode();
                    return _goals.at(i)->getOverBlock();
                }
            }
        }
            break;
        case GOAL_MATCH:
        case GOAL_UNDERBLOCKS: {
            gnodes.push_back(_goals);
        }
            break;
        case GOAL_ICE: {
            gnodes.push_back(std::vector<TableNode*>());
            graph->getBoundryNodes(gnodes.at(0), _goals,  [] (TableNode* node) -> bool {
                return node && node->isActive() && node->getType() == TableNode::EMPTY;
            });
        }
            break;
        default:
            return nullptr;
            break;
    }
    
    for (auto& gnode : gnodes ) {
        std::sort(gnode.begin(),gnode.end());
    }
    
    if (check_targets) {
        std::vector<TableNode*> goals = _goals;
        std::vector<TableNode*> targets;
        getTargets(targets);
        std::vector<std::vector<TableNode*>> tnodes;
        getTargetDropNodes(targets, tnodes);
        if (tnodes.size() == 0) return nullptr;
        // discard goals in target drop nodes
        for ( auto& tnode : tnodes ) {
            for ( size_t i = 0; i < goals.size();)
                if (std::find(tnode.begin(), tnode.end(), goals.at(i)) != tnode.end()) {
                    goals.erase(goals.begin()+ i);
                }
                else {
                    i++;
                }
        }
        if (goals.size() == 0)
            return nullptr;
        float best_distance = MAXFLOAT;
        TableNode* best_node[3] = { nullptr, nullptr, nullptr };
        for ( auto goal : goals ) {
            if (!goal->isActive()) continue;
            for ( uint8_t i = 0; i < 8; i+=2 ) {
                TableNode* a = goal->getAdjacentActiveNode(TableNode::TableDir(i));
                TableNode* b = goal;
                if (!func(a) || a->getOverBlock()->getColor() == b->getOverBlock()->getColor()) continue;
                graph->swapBlocks(a, b);
                for ( auto& tnode: tnodes ) {
                    for ( auto& node: tnode ) {
                        if ( node == a ) {
                            booster_hint_1 = a;
                            booster_hint_2 = b;
                        }
                        float dist = a->getTablePosition().distance(node->getTablePosition());
                        if ( best_distance > dist ) {
                            best_distance = dist;
                            best_node[0] = a; best_node[1] = b; best_node[2] = node;
                        }
                    }
                }
                graph->swapBlocks(a, b);
                if ( booster_hint_1 || booster_hint_2 ) return booster_hint_1;
            }
        }
        if (best_node[0] != nullptr && best_node[1] != nullptr && best_node[2] != nullptr &&
            best_distance < best_node[1]->getTablePosition().distance(best_node[2]->getTablePosition())) {
            booster_hint_1 = best_node[0]; booster_hint_2 = best_node[1];
            return booster_hint_1;
        }
    }
    for ( auto node : nodes ) {
        for ( uint8_t i = 0; i < 8; i+=2 ) {
            TableNode* a = node->getAdjacentActiveNode(TableNode::TableDir(i));
            TableNode* b = node;
            if (!func(a) || a->getOverBlock()->getColor() == b->getOverBlock()->getColor()) continue;
            graph->swapBlocks(a, b);
            std::vector<TableHint*> hints;
            auto match1 = graph->checkForMatchs(b, false);
            auto match2 = graph->checkForMatchs(a, false);
            if (match1 || match2) {
                graph->swapBlocks(a, b);
                delete match1;
                delete match2;
                continue;
            }
            graph->checkForDirectionHint(node, hints);
            for ( auto& hint : hints ) {
                for (auto gnode : gnodes) {
                    std::vector<TableNode*> anodes(nodes.size());
                    std::vector<TableNode*> bnodes;
                    getMatchNodes(bnodes, hint->match);
                    std::vector<TableNode*> inodes(nodes.size());
                    auto iter = std::set_intersection(gnode.begin(), gnode.end(),
                                                      bnodes.begin(), bnodes.end(),
                                                      inodes.begin());
                    
                    inodes.resize(iter - inodes.begin());
                    if ( inodes.size() ) {
                        booster_hint_1 = a;
                        booster_hint_2 = b;
                    }
                }
                delete hint;
            }
            graph->swapBlocks(a, b);
            if ( booster_hint_1 || booster_hint_2 ) return node->getOverBlock();
        }
    }
    return nullptr;
}

TableHint* TableBot::getTargetHint ( )
{
    std::vector<TableNode*> goals = _goals;
    std::vector<TableNode*> targets;
    getTargets(targets);
    std::vector<std::vector<TableNode*>> tnodes;
    getTargetDropNodes(targets, tnodes);
    if (tnodes.size() == 0) return nullptr;
    // discard goals in target drop nodes
    for ( auto& tnode : tnodes ) {
        for ( size_t i = 0; i < goals.size();)
            if (std::find(tnode.begin(), tnode.end(), goals.at(i)) != tnode.end()) {
                goals.erase(goals.begin()+ i);
            }
            else {
                i++;
            }
    }
    if (goals.size() == 0)
        return nullptr;
    TableHint* best_hint = nullptr;
    float best_distance = MAXFLOAT;
    TableNode* best_node = nullptr;
    for ( auto& hint : _hints ) {
        //skip hints that dont contain a goal
        auto iter = std::find(goals.begin(), goals.end(), hint->match->nodes.front());
        if (iter == goals.end()) continue;
        // find goals that will intersect with target nodes
        for ( auto& tnode: tnodes ) {
            for ( auto& node: tnode ) {
                if ( node == hint->source )
                    return hint;
                // if goal doesn't intersect with target node find the closest.
                float dist = node->getTablePosition().distance(hint->source->getTablePosition());
                if ( dist < best_distance ) {
                    best_hint = hint;
                    best_distance = dist;
                    best_node = node;
                }
            }
        }
    }
    if ( best_hint && best_distance > best_node->getTablePosition().distance(best_hint->match->nodes.front()->getTablePosition()) ) {
        _stats_lable->_hint_label->setString("TACTIC: Target");
        return best_hint;
    }
    return nullptr;
}

TableHint* TableBot::getLowPriorityHint ( )
{
    std::vector<TableNode*> bnodes;
    getBlockedNodes( bnodes );
    
    std::vector<TableNode*> snodes;
    getShakeNodes( snodes );
    
    std::vector<int> priortity(_hints.size(),0);
    for ( uint8_t i = 0; i < _hints.size(); i++ ) {
        auto hint = _hints.at(i);
        auto hinti = _hint_intersect.at(i);
        
        std::vector<TableNode*> inodes(MAX(hinti.size(), bnodes.size()));
        auto iter = std::set_intersection(hinti.begin(), hinti.end(),
                                          bnodes.begin(), bnodes.end(),
                                          inodes.begin());
        inodes.resize(iter - inodes.begin());
        priortity.at(i) += inodes.size();
        
        inodes.resize( MAX(hint->match->nodes.size(), snodes.size()) );
        iter = std::set_intersection(hint->match->nodes.begin(), hint->match->nodes.end(),
                                     snodes.begin(), snodes.end(),
                                     inodes.begin());
        inodes.resize(iter - inodes.begin());
        priortity.at(i) += inodes.size();
        
        for ( auto block : hint->match->blocks ) {
            if ( block->getFallState() == STATIC ) {
                priortity.at(i)++;
            }
        }
    }
    
    int8_t idx = -1;
    for ( uint8_t i = 0; i < priortity.size(); i++ ) {
        if ( priortity.at(i) != 0 && (idx < 0 || priortity.at(i) > priortity.at(idx) )) {
            idx = i;
        }
    }
    
    if ( idx == -1 ) return nullptr;
    BOTLOG ("Found Low Priority Hint");
    _stats_lable->_hint_label->setString("TACTIC: Low Priority");
    return _hints.at(idx);
}

TableHint* TableBot::getDropHint ( uint8_t opt )
{
    std::vector<TableNode*> gnodes;
    getGoalDropNodes(_goals, gnodes);
    TableGoal goal = _scene->getRule();

    // return if eye is visible
    if (goal == GOAL_DISPEL) {
        for ( auto& i : _goals ) {
            if (i->isActive()) {
                return nullptr;
            }
        }
    }
    std::vector<int> priortity;

    for ( uint8_t i = 0; i < _hints.size(); i++ ) {
        auto hint = _hints.at(i);
        auto hinti = _hint_intersect.at(i);
        std::vector<TableNode*> inodes(MAX(hinti.size(), gnodes.size()));
        auto iter = std::set_intersection(hinti.begin(), hinti.end(),
                                          gnodes.begin(), gnodes.end(),
                                          inodes.begin());
        inodes.resize(iter - inodes.begin());
        int8_t counter = 0;
        switch (opt) {
            case 2:
            case 0: {
                counter = inodes.size();
                if (counter == 0) {
                    for ( auto goal : _goals ) {
                        if (inodes.size() == 0 && hint->match->in(goal) &&
                            std::find(hinti.begin(), hinti.end(), hint->source) != hinti.end()) {
                            counter = 1;
                        }
                    }
                }
            }
                break;
            case 1: {
                counter = -inodes.size();
                if (counter == 0)
                    counter = INT8_MIN;
                for ( auto block : hint->match->blocks ) {
                    if ( block == hint->source->getOverBlock() && hint->match->match_type != TableMatch::MATCH_THREE )
                        continue;
                    /*if ( block->getType() == FRUIT ) {
                        counter++;
                    }*/
                }
            }
                break;
            default:
                CCASSERT(false, "option not supported");
                break;
        }
        priortity.push_back(counter);

    }
    int8_t min = INT8_MAX; int8_t max = INT8_MIN;
    
    for ( auto num : priortity ) {
        min = MIN(min,num);
        max = MAX(max,num);
    }
    if (min == 0 && max == 0) return nullptr;
    
    int8_t idx = -1;
    for ( uint8_t i = 0; i < priortity.size(); i++ ) {
        if ( idx < 0 ||
            priortity.at(i) > priortity.at(idx) ||
            ( priortity.at(i) == priortity.at(idx) &&
             (( opt == 0 && _hints.at(i)->match->blocks.size() > _hints.at(idx)->match->blocks.size() ) ||
              ( opt == 1 && _hints.at(i)->match->blocks.size() < MIN(1,_hints.at(idx)->match->blocks.size())) ||
              ( opt == 2 && _hint_intersect.at(i).size() < _hint_intersect.at(idx).size()))))
            {
            idx = i;
        }
    }
    BOTLOG ("Found Drop Hint");
    _stats_lable->_hint_label->setString("TACTIC: Drop");
    return (_hints.at(idx));
}

void TableBot::getMatchNodes(std::vector<TableNode*>& out_nodes, TableMatch* match)
{
    if ( match == nullptr ) return;
    TableLayer* table = _scene->getTable();
    std::vector<TableNode*> new_nodes;

    new_nodes.insert(new_nodes.begin(), match->nodes.begin(), match->nodes.end());
    if (match->isMatchTwo()) {
        BlockType type = NORMAL;
        BlockColor color = NONE;
        if (match->match_type == TableMatch::MATCH_OMNI ||
            match->match_type == TableMatch::MATCH_TWO_MANA) {
            for ( auto block : match->blocks ) {
                if (block->getType() != OMNI && block->getType() != MANA) {
                    type = block->getType(); color = block->getColor();
                }
            }
        }
        table->getDestructNodes(new_nodes, match->nodes.at(1),
                                table->getDestructPattern(match), color, type, match->nodes.at(1)->isRestricted());
    } else {
        for ( uint8_t j = 0; j < match->nodes.size(); j++ ) {
            table->getDestructNodes(new_nodes, match->nodes.at(j),
                                    table->getDestructPattern(match->blocks.at(j)), NONE, NORMAL, match->nodes.at(j)->isRestricted() );
        }
        table->getDestructNodes(new_nodes, match->nodes.at(0), table->getDestructPattern(table->getInsertBlockType(match)), NONE, NORMAL, match->nodes.at(0)->isRestricted());
    }
    /*std::sort(new_nodes.begin(), new_nodes.end());
    auto iter = std::unique(new_nodes.begin(), new_nodes.end());
    new_nodes.resize( std::distance(new_nodes.begin(),iter) );
    iter = std::remove(new_nodes.begin(), new_nodes.end(),nullptr);
    new_nodes.resize( std::distance(new_nodes.begin(),iter) );*/
    
    for (uint16_t i =  match->nodes.size(); i < new_nodes.size();) {
        auto iter = std::find(new_nodes.begin(), new_nodes.end(), new_nodes.at(i));
        if ( iter - new_nodes.begin() < i || new_nodes.at(i) == nullptr ) {
            new_nodes.erase(new_nodes.begin() + i);
            continue;
        }
        TableBlock* block = new_nodes.at(i)->getOverBlock();
        if (block) {
            TableBlockPrison* prisoner = dynamic_cast<TableBlockPrison*>(block);
            if (prisoner)
                block = prisoner->getPrisoner();
            std::vector<TableNode*> n_nodes;
            table->getDestructNodes( n_nodes, new_nodes.at(i),
                                     table->getDestructPattern(block), block->getColor(), block->getType(),
                                     new_nodes.at(i)->isRestricted() );
            
            auto offset = new_nodes.size();
            new_nodes.resize( new_nodes.size() + n_nodes.size() );
            auto jter = std::set_difference( n_nodes.begin(), n_nodes.end(),
                                             new_nodes.begin(), new_nodes.end(),
                                             new_nodes.begin() + offset );
            new_nodes.resize( std::distance(new_nodes.begin(),jter) );
        }
        i++;
    }
    for (uint16_t i = 0; i < new_nodes.size();) {
        TableNode* node = new_nodes.at(i);
        if ( !node->getOverBlock() || node->getOverBlock()->isInvinsible() )
            new_nodes.erase(new_nodes.begin() + i);
        else
            i++;
    }
    
    std::sort(new_nodes.begin(), new_nodes.end());
    
    out_nodes.insert(out_nodes.end(), new_nodes.begin(), new_nodes.end());
}
void TableBot::getHints ()
{
    TableLayer* table = _scene->getTable();

    const std::vector<TableNode*>& nodes = table->getGraph()->getNodes();
    
    for( auto hint : _hints ) {
        delete hint;
    }
    _hints.clear();
    _hint_intersect.clear();
    
    for (auto i : nodes ) {
        table->getGraph()->checkForDirectionHint(i, _hints);
    }
    _hint_intersect.resize(_hints.size());
    for ( uint8_t i = 0; i < _hints.size(); i++) {
        TableHint* hints[2] = { _hints.at(i), nullptr };
        for ( auto other_hint : _hints ) {
            if ( hints[0]->source == other_hint->match->nodes[0] &&
                 other_hint->source == hints[0]->match->nodes[0] &&
                 !hints[0]->match->isMatchTwo() ) {
                hints[1] = other_hint;
            }
        }
        std::vector<TableNode*>& dnodes = _hint_intersect.at(i);
        for ( uint8_t h = 0; h < 2; h++ ) {
            if ( hints[h] == nullptr ) break;
            getMatchNodes(dnodes, hints[h]->match);
        }
        std::sort(dnodes.begin(), dnodes.end());
        auto iter = std::unique(dnodes.begin(), dnodes.end());
        dnodes.resize( std::distance(dnodes.begin(),iter) );
    }
}

void TableBot::tableReady()
{
    TableLayer* table = _scene->getTable();
    TableGoal goal = _scene->getRule();
    TableNode* node = nullptr;
    TableHint* hint = nullptr;
    float delay = 1.0f;
    _goals.clear();
    
    switch (_scene->getMode()) {
        case TableScene::NORMAL:
            booster_hint_1 = nullptr; booster_hint_2 = nullptr;
            break;
        case TableScene::PORTAL:
            table->setTouchNodes(nullptr, booster_hint_1,false);
            table->nodeTouched(TableNodeSprite::get(booster_hint_1));
            _scene->scheduleOnce([this,table] (float dt) {
                table->setTouchNodes(nullptr, booster_hint_2,false);
                table->nodeTouched(TableNodeSprite::get(booster_hint_2));
            }, delay, "Bot Touch Node");
            delay *= 2.0f;
            break;
        case TableScene::SWAP:
            table->setTouchNodes(booster_hint_1,booster_hint_2,true);
            table->blockMoved();
            break;
        case TableScene::PAINT:
        case TableScene::HAMMER:
        case TableScene::DISPEL:
            table->setTouchNodes(nullptr, booster_hint_1,false);
            table->nodeTouched(TableNodeSprite::get(booster_hint_1));
            break;
        default:
            
            break;
    }
    if (TableScene::isBooster(_scene->getMode())) {
        _scene->scheduleOnce([this,table] (float dt) {
                _scene->getHUD()->buttonPressed(TableScene::BOOSTER_YES);
        }, delay, "Bot Touch Button");
        return;
    }

    getHints ();
    getGoals (_goals);

    switch (goal) {
        case GOAL_UNDERBLOCKS:
            if (!hint) hint = getMatchFives     ( );
            if (!hint) hint = getDestructHint   ( );
            if (!hint) hint = getBlockedHint    ( );
            if (!hint && getBoosterHint         ( nullptr, &hint, 1) ) { if (!hint) return; }
            if (!hint) hint = getProximityHint  ( );
            if (!hint) hint = getFruitHint      ( );
            if (!hint) hint = getClosestHint    ( 2 );
            if (!hint && getBoosterHint         ( nullptr, &hint, 0) ) { if (!hint) return; }
            if (!hint) hint = getLowPriorityHint( );
            break;
        case GOAL_DISPEL: {
            TableBlock* block = nullptr;
            if (!hint && getBoosterHint         (&block, &hint)) { if (!hint) return; }
            if (block && !block->getNode()->isActive()) { hint = getDropHint( 2 ); }
            else if (block) { return; }
            if (!hint) hint = getDropHint       ( 1 );
            if (!hint) hint = getFruitHint      ( );
        }
            break;
        case GOAL_COLLECT:
            if (!hint) hint = getMatchFives     ( );
            if (!hint) hint = getBlockedHint    ( );
            if (!hint) hint = getDestructHint   ( );
            if (!hint) hint = getDropHint       ( 0 );
            if (!hint && getBoosterHint         ( nullptr, &hint, 1 )) { if (!hint) return; }
            if (!hint) hint = getTargetHint     ( );
            if (!hint) hint = getProximityHint  ( );
            if (!hint) hint = getFruitHint      ( );
            if (!hint && getBoosterHint         ( nullptr, &hint, 0 )) { if (!hint) return; }
            if (!hint) hint = getClosestHint    ( 2 );
            if (!hint) hint = getLowPriorityHint( );
            break;
        case GOAL_MATCH:
            if (!hint) hint = getMatchFives     ( );
            if (!hint) hint = getMatchHint      ( );
            if (!hint) hint = getDestructHint   ( );
            if (!hint && getBoosterHint         ( nullptr, &hint, 1 )) { if (!hint) return; }
            if (!hint) hint = getFruitHint      ( );
            if (!hint && getBoosterHint         ( nullptr, &hint, 0 )) { if (!hint) return; }
            if (!hint) hint = getLowPriorityHint( );
            break;
        case GOAL_JUICE:
            if (!hint) hint = getMatchFives     ( );
            if (!hint && getBoosterHint         ( nullptr, &hint, 1 )) { if (!hint) return; }
            if (!hint) hint = getDestructHint   ( );
            if (!hint) hint = getFruitHint      ( );
            if (!hint) hint = getBlockedHint    ( );
            if (!hint && getBoosterHint         ( nullptr, &hint, 0 )) { if (!hint) return; }
            if (!hint) hint = getLowPriorityHint( );
            break;
        case GOAL_ICE:
            if (!hint) hint = getMatchFives     ( );
            if (!hint) hint = getDestructHint   ( );
            if (!hint) hint = getPredictiveHint ( 3 );
            if (!hint) hint = getBlockedHint    ( );
            if (!hint && getBoosterHint         ( nullptr, &hint, 1 )) { if (!hint) return; }
            if (!hint) hint = getFruitHint      ( );
            if (!hint) hint = getPredictiveHint ( 1 );
            if (!hint && getBoosterHint         ( nullptr, &hint, 0 )) { if (!hint) return; }
            if (!hint) hint = getProximityHint  ( );
            if (!hint) hint = getLowPriorityHint( );
            break;
        case GOAL_FRUIT:
            if (!hint) hint = getMatchFives     ( );
            if (!hint) hint = getDestructHint   ( );
            if (!hint) hint = getMatchHint      ( );
            if (!hint && getBoosterHint         ( nullptr, &hint, 1 )) { if (!hint) return; }
            if (!hint) hint = getBlockedHint    ( );
            if (!hint) hint = getFruitHint      ( );
            if (!hint) hint = getDropHint       ( 1 );
            if (!hint && getBoosterHint         ( nullptr, &hint, 0 )) { if (!hint) return; }
            if (!hint) hint = getProximityHint  ( );
            if (!hint) hint = getLowPriorityHint( );
            break;
        case GOAL_SCORE_NOFRUIT: break;
        case GOAL_SCORE: break;
        default:
            break;
    }
    
    if (!hint) {
        BOTLOG ("No Hint Found");
        _stats_lable->_hint_label->setString("TACTIC: None");
        hint = table->getHint();
    }
    node = hint->match->nodes.front();
    if (node == hint->source)
        node = hint->match->nodes.back();
    table->setTouchNodes(node,hint->source,true);
    table->blockMoved();
}

void TableBot::begin ( std::string& level_name )
{
    ValueMap level_stats = FileUtils::getInstance()->getValueMapFromFile(FileUtils::getInstance()->getWritablePath() + "level_stats.plist");
    auto iter = level_stats.find(level_name);
    if (iter != level_stats.end()) {
        ValueVector& vec = iter->second.asValueVector();
        if (vec.size() >= 30) {
            utils::captureScreen(nullptr, FileUtils::getInstance()->getWritablePath() + level_name + ".png");
        }
    }
}
TableStats* TableStats::create( std::string& level )
{
    TableStats* pRet = new (std::nothrow) TableStats();
    if (pRet && pRet->init(level)) {
        return pRet;
    }
    delete pRet;
    return nullptr;
}


bool TableStats::init( std::string& level )
{
    if (!Node::init()) return false;

    std::vector<std::string> data;
    uint32_t avg_points = 0;
    int32_t highest_points = 0;
    int32_t lowest_points = 0;
    int32_t avg_moves = 0;
    int32_t highest_moves = 0;
    int32_t lowest_moves = 0;
    int32_t wins = 0;
    int32_t loses = 0;
    int32_t median_moves = 0;
    int32_t median_score = 0;
    float difficulty = 0.0f;
    
    
    ValueMap level_stats = FileUtils::getInstance()->getValueMapFromFile(FileUtils::getInstance()->getWritablePath() + "level_stats.plist");
    auto iter = level_stats.find(level);
    auto jter = level_stats.find("Difficulty");
    if (jter != level_stats.end()) {
        auto& bter = jter->second.asValueMap();
        jter = bter.find(level);
        if (jter != bter.end()) {
            if (Game::getInstance()->getProfile()->getDebugFlag(Profile::CLEAR_STATS)) {
                bter.erase(jter);
            } else {
                difficulty = jter->second.asFloat();
            }
        }
    }
    
    std::vector<int32_t> median_score_list;
    std::vector<int32_t> median_moves_list;

    if (iter != level_stats.end()) {
        if (Game::getInstance()->getProfile()->getDebugFlag(Profile::CLEAR_STATS)) {
            level_stats.erase(iter);
        } else {
            ValueVector& vec = (*iter).second.asValueVector();
            uint16_t counter = 0;
            for ( auto& j : vec ) {
                ValueVector& vec2 = j.asValueVector();
                if ( !vec2.at(2).asBool() ) {
                    loses++;
                    continue;
                }
                /*if ( vec2.at(1).asInt() == 0 ) {
                    continue;
                }*/
                wins++;
                counter++;
                int32_t score = vec2.at(0).asInt();
                median_score_list.push_back(score);
                avg_points += score;
                
                int32_t moves = vec2.at(1).asInt();
                median_moves_list.push_back(moves);
                avg_moves += moves;
            }
            if (counter) {
                avg_points /= counter;
                avg_moves /= counter;
            }
        }
    }
    
    if (Game::getInstance()->getProfile()->getDebugFlag(Profile::CLEAR_STATS)) {
        Game::getInstance()->getProfile()->setDebugFlag(Profile::CLEAR_STATS,false);
        FileUtils::getInstance()->writeToFile(level_stats, FileUtils::getInstance()->getWritablePath() + "level_stats.plist");
    }
    
    if ( median_score_list.size() && median_moves_list.size()) {
        std::sort(median_score_list.begin(), median_score_list.end());
        std::sort(median_moves_list.begin(), median_moves_list.end());
        lowest_points = *(median_score_list.begin());
        highest_points = *(median_score_list.end()-1);
        lowest_moves = *(median_moves_list.begin());
        highest_moves = *(median_moves_list.end()-1);
        median_score = median_score_list.at(median_score_list.size()/2);
        median_moves = median_moves_list.at(median_moves_list.size()/2);
    }

    for ( uint8_t i = 0; i < 6; i++ )
    {
        Label** label = nullptr;
        std::string data;
        Color3B color;
        switch (i) {
            case 0: label = &_moves_label; data = "MOVES: HIGH-" + std::to_string(highest_moves) +
                " AVG-" + std::to_string(avg_moves) +
                " MEDIAN-" + std::to_string(median_moves) +
                " LOW-" + std::to_string(lowest_moves); color = Color3B::ORANGE; break;
            case 1: label = &_score_label; data = "SCORE: HIGH-" + std::to_string(highest_points) +
                " AVG-" + std::to_string(avg_points) +
                " MEDIAN-" + std::to_string(median_score) +
                " LOW-" + std::to_string(lowest_points); color = Color3B::GREEN; break;
            case 2: label = &_winlose_label; data = "WINS-" + std::to_string(wins) +
                " LOSES-" + std::to_string(loses); color = Color3B::RED; break;
            case 3: label = &_difficulty_label; data = "DIFFICULTY: " + std::to_string(difficulty);
                color = Color3B::RED; break;
            case 4: label = &_level_name_label; data = "LEVEL: " + level;
                color = Color3B::YELLOW; break;
            case 5: label = &_hint_label; data = "TACTIC: ";
                color = Color3B::MAGENTA; break;
            default: break;
        }
        *label = Label::createWithTTF(data,"fonts/NOTMK.TTF", 60);
        (*label)->enableOutline(Color4B::BLACK,4);
        (*label)->setScale(0.5f);
        (*label)->setColor(color);
        (*label)->setAnchorPoint(Vec2(0.0f,0.5f));
        //(*label)->setScale(scaleFactor);
        (*label)->setPosition(Vec2(0.0f,i * 32 + (*label)->getContentSize().height * 0.5f));
        addChild(*label);
        
    }
    return true;
}

bool TableBot::getBoosterHint( TableBlock** block, TableHint** hint, uint8_t opt )
{
    TableScene::TableMode booster = TableScene::NORMAL;
    bool pass = false;
    for (auto button : _boosters) {
        if (button->isEnabled() && _scene->isBooster(TableScene::TableMode(button->getTag())) ) {
            booster = TableScene::TableMode(button->getTag());
            switch (booster) {
                case TableScene::PAINT:
                    pass = getPaintHint(block, hint);
                    break;
                case TableScene::SWAP:
                    pass = getSwapHint(block);
                    break;
                case TableScene::SHUFFLE:
                    if (opt == 0)
                        pass = true;
                    break;
                case TableScene::PORTAL:
                    pass = getTeleportHint(block);
                    break;
                case TableScene::DISPEL:
                    pass = getDispelHint(block);
                    break;
                case TableScene::HAMMER:
                    pass = getHammerHint(block, opt);
                    break;
                default:
                    booster = TableScene::NORMAL;
                    break;
            }
        }
        if (!pass) {
            booster = TableScene::NORMAL;
        } else {
            _stats_lable->_hint_label->setString("TACTIC: Booster");
            break;
        }
    }
    if (booster != TableScene::NORMAL) {
        BOTLOG ("Found Booster Hint");
        _scene->getHUD()->buttonPressed(booster);
    }
    return pass;
    
}

uint16_t TableBot::gameOver( std::string& level, uint32_t points, bool success )
{
    
    ValueMap level_stats = FileUtils::getInstance()->getValueMapFromFile(FileUtils::getInstance()->getWritablePath() + "level_stats.plist");
    auto iter = level_stats.find(level);
    if (iter == level_stats.end()) {
        iter = level_stats.insert(iter, std::pair<std::string, Value>(level,Value(ValueVector())));
    }
    auto jter = level_stats.find("Difficulty");
    if (jter == level_stats.end()) {
        jter = level_stats.insert(jter, std::pair<std::string, Value>("Difficulty",Value(ValueMap())));
    }
    auto& bter = jter->second.asValueMap();
    jter = bter.find(level);
    if (jter == bter.end()) {
        jter = bter.insert(jter, std::pair<std::string, Value>(level,Value(0.0f)));
    }
    
    ValueVector& vec = (*iter).second.asValueVector();
    
    float loses = 0.0f;
    float wins = 0.0f;
    
    for ( auto& j : vec ) {
        ValueVector& vec2 = j.asValueVector();
        if ( !vec2.at(2).asBool() )  {
            loses++;
        } else {
            wins++;
        }
    }
    jter->second = wins/loses;
    ValueVector new_vec;
    new_vec.push_back(Value((int)points));
    new_vec.push_back(Value((int)_moves));
    new_vec.push_back(Value(success));
    vec.push_back(Value(new_vec));

    
    FileUtils::getInstance()->writeToFile(level_stats, FileUtils::getInstance()->getWritablePath() + "level_stats.plist");
    return wins + loses;
}
