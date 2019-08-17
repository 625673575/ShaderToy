#include "Node.h"

namespace ShaderNodeEditor {
    size_t getInnerParamCount(const std::string& p) {
        size_t num = std::count(p.begin(), p.end(), '}');
        return num;
    }
    void Graph::evaluate(const size_t root_node) {
        // this function does a depth-first evaluation of the graph
        // the nodes are evaluated post-order using two stacks.
        std::stack<std::string> eval_stack;
        std::stack<size_t> preorder;
        std::stack<size_t> postorder;

        preorder.push(root_node);

        while (!preorder.empty())
        {
            const size_t node = preorder.top();
            preorder.pop();

            postorder.push(node);

            for (const size_t edge : edges_from_node_[node])
            {
                const size_t neighbor = edges_[edge].opposite(node);
                assert(neighbor != root_node);
                preorder.push(neighbor);
            }
        }

        std::stack<std::string> token;
        // unwind the stack and call each operation along the way
        while (!postorder.empty())
        {
            const size_t node = postorder.top();
            postorder.pop();
            switch (nodes_[node]->type)
            {
            case Node_Number:
                token.push("%N");
                OutputDebugStringA(" Number ");
                break;
            case Node_NumberExpression:
                //    if (nodes_[node]->Id.params.empty()) {
                //    OutputDebugStringA((" Expression Param Empty" + std::to_string(nodes_[node]->number.fVal[0])).c_str());
                //}
                //    else {
                //    OutputDebugStringA((" Expression " + std::to_string(nodes_[node]->Id.params[0].value.fVal[0])).c_str());
                //}
                break;
            case Node_Operation: {
                OutputDebugStringA(" Operation:");
                auto stackSize = token.size();
                std::string snippet = nodes_[node]->Interpret();
                //如果不包含参数,则直接压栈,否则取参数在压栈
                size_t paramCount = getInnerParamCount(snippet);
                if (paramCount > 0) {
                    //参数多于栈中的,则需要从Id.params里面找添补的
                    if (paramCount > stackSize) {

                    }
                    //参数小于栈中的,则取后面的几个,同时参数顺序按照先进的顺序排列
                    else {
                        size_t pc = paramCount;
                        std::vector<std::string> par(pc);
                        while (pc > 0) {
                            par[--pc] = token.top(); token.pop();
                        }
                        //替换参数后压栈
                    }
                    token.push("转化后的snippet");
                }
                else {
                    //snippet还是要处理替换Constant,Variable之类的

                    token.push(snippet);
                }

                for (auto& v : nodes_[node]->Id.params) {
                    OutputDebugStringA(std::to_string(v.value.fVal[0]).c_str());
                }
                OutputDebugStringA(nodes_[node]->Interpret().c_str());
            }
                                 break;
            case Node_Output:
                OutputDebugStringA(" Output ");
                break;
            default:
                assert("Invalid enum value!");
            }
        }
        OutputDebugStringA("\n");
    }
}
