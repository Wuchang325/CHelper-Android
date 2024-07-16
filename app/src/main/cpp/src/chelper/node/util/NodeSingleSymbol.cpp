//
// Created by Yancey on 2023/12/19.
//

#include "NodeSingleSymbol.h"

namespace CHelper::Node {

    static std::shared_ptr<NormalId> getNormalId(char symbol, const std::optional<std::string> &description) {
        std::string name;
        name.push_back(symbol);
        return NormalId::make(name, description);
    }

    NodeSingleSymbol::NodeSingleSymbol(const std::optional<std::string> &id,
                                       const std::optional<std::string> &description,
                                       char symbol,
                                       bool isAddWhitespace)
        : NodeBase(id, description, false),
          symbol(symbol),
          normalId(getNormalId(symbol, description)),
          isAddWhitespace(isAddWhitespace) {}

    NodeType *NodeSingleSymbol::getNodeType() const {
        return NodeType::SINGLE_SYMBOL.get();
    }

    ASTNode NodeSingleSymbol::getASTNode(TokenReader &tokenReader, const CPack *cpack) const {
        ASTNode symbolNode = tokenReader.readSymbolASTNode(this);
        std::shared_ptr<ErrorReason> errorReason;
        if (HEDLEY_UNLIKELY(symbolNode.isError())) {
            if (HEDLEY_LIKELY(symbolNode.tokens.isEmpty())) {
                return ASTNode::simpleNode(this, symbolNode.tokens, ErrorReason::incomplete(symbolNode.tokens, FormatUtil::format("命令不完整，需要符号{0}", symbol)));
            } else {
                return ASTNode::simpleNode(this, symbolNode.tokens, ErrorReason::typeError(symbolNode.tokens, FormatUtil::format("类型不匹配，需要符号{0}，但当前内容为{1}", symbol, symbolNode.tokens.toString())));
            }
        }
        std::string_view str = symbolNode.tokens.toString();
        if (HEDLEY_LIKELY(str.length() == 1 && str[0] == symbol)) {
            return symbolNode;
        }
        return ASTNode::simpleNode(this, symbolNode.tokens, ErrorReason::contentError(symbolNode.tokens, FormatUtil::format("内容不匹配，正确的符号为{0}，但当前内容为{1}", symbol, str)));
    }

    bool NodeSingleSymbol::collectSuggestions(const ASTNode *astNode,
                                              size_t index,
                                              std::vector<Suggestions> &suggestions) const {
        if (HEDLEY_LIKELY(astNode->tokens.getStartIndex() != index)) {
            return true;
        }
        suggestions.push_back(Suggestions::singleSuggestion({index, index, isAddWhitespace, normalId}));
        return true;
    }

}// namespace CHelper::Node