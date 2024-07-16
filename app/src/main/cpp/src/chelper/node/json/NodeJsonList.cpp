//
// Created by Yancey on 2024/2/28.
//

#include "NodeJsonList.h"
#include "../util/NodeList.h"
#include "../util/NodeSingleSymbol.h"
#include "NodeJsonElement.h"

namespace CHelper::Node {

    static std::unique_ptr<NodeBase> nodLeft = std::make_unique<NodeSingleSymbol>(
            "JSON_LIST_LEFT", "JSON列表左括号", '[');
    static std::unique_ptr<NodeBase> nodeRight = std::make_unique<NodeSingleSymbol>(
            "JSON_LIST_RIGHT", "JSON列表右括号", ']');
    static std::unique_ptr<NodeBase> nodeSeparator = std::make_unique<NodeSingleSymbol>(
            "JSON_LIST_SEPARATOR", "JSON列表分隔符", ',');
    static std::unique_ptr<NodeBase> nodeAllList = std::make_unique<NodeList>(
            "JSON_OBJECT", "JSON对象",
            nodLeft.get(), NodeJsonElement::getNodeJsonElement(), nodeSeparator.get(), nodeRight.get());

    NodeJsonList::NodeJsonList(const std::optional<std::string> &id,
                               const std::optional<std::string> &description,
                               std::string data)
        : NodeBase(id, description, false),
          data(std::move(data)) {}

    void NodeJsonList::init(const std::vector<std::unique_ptr<NodeBase>> &dataList) {
        for (const auto &item: dataList) {
            if (HEDLEY_UNLIKELY(item->id == data)) {
                nodeList = std::make_unique<NodeList>("JSON_LIST", "JSON列表",
                                                      nodLeft.get(), item.get(),
                                                      nodeSeparator.get(), nodeRight.get());
                return;
            }
        }
        Profile::push(ColorStringBuilder()
                              .red("linking contents to ")
                              .purple(data)
                              .build());
        Profile::push(ColorStringBuilder()
                              .red("failed to find node id")
                              .normal(" -> ")
                              .purple(data)
                              .build());
        throw std::runtime_error(ColorStringBuilder()
                                         .red("unknown node id")
                                         .normal(" -> ")
                                         .purple(id.value_or("UNKNOWN"))
                                         .red(" (in node \"")
                                         .purple(data)
                                         .red("\")")
                                         .build());
    }

    NodeType *NodeJsonList::getNodeType() const {
        return NodeType::JSON_LIST.get();
    }

    ASTNode NodeJsonList::getASTNode(TokenReader &tokenReader, const CPack *cpack) const {
        if (HEDLEY_UNLIKELY(nodeList == nullptr)) {
            return getByChildNode(tokenReader, cpack, nodeAllList.get(), ASTNodeId::NODE_JSON_ALL_LIST);
        }
        tokenReader.push();
        ASTNode result1 = nodeList->getASTNode(tokenReader, cpack);
        if (HEDLEY_LIKELY(!result1.isError())) {
            return ASTNode::andNode(this, {std::move(result1)}, tokenReader.collect());
        }
        size_t index1 = tokenReader.index;
        tokenReader.restore();
        tokenReader.push();
        ASTNode result2 = getByChildNode(tokenReader, cpack, nodeAllList.get(), ASTNodeId::NODE_JSON_ALL_LIST);
        size_t index2 = tokenReader.index;
        tokenReader.restore();
        tokenReader.push();
        tokenReader.index = result1.isError() ? index2 : index1;
        return ASTNode::orNode(this, {std::move(result1), std::move(result2)}, tokenReader.collect());
    }

    bool NodeJsonList::collectSuggestions(const ASTNode *astNode,
                                          size_t index,
                                          std::vector<Suggestions> &suggestions) const {
        return astNode->id == ASTNodeId::NODE_JSON_ALL_LIST;
    }

    CODEC_NODE(NodeJsonList, data)

}// namespace CHelper::Node