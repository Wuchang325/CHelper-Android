//
// Created by Yancey on 2023/11/11.
//

#include "NodeBlock.h"
#include "../util/NodeSingleSymbol.h"
#include "NodeString.h"

namespace CHelper::Node {

    static std::unique_ptr<Node::NodeSingleSymbol> nodeBlockStateLeftBracket = std::make_unique<Node::NodeSingleSymbol>(
            "BLOCK_STATE_LEFT_BRACKET", "方块状态左括号", '[');

    void NodeBlock::init(const CPack &cpack) {
        blockIds = cpack.blockIds;
        nodeBlockId = std::make_shared<NodeNamespaceId>("BLOCK_ID", "方块ID", "blocks", true);
        nodeBlockId->init(cpack);
    }

    NodeType *NodeBlock::getNodeType() const {
        return NodeType::BLOCK.get();
    }

    ASTNode NodeBlock::getASTNode(TokenReader &tokenReader, const CPack *cpack) const {
        tokenReader.push();
        ASTNode blockId = getByChildNode(tokenReader, cpack, nodeBlockId.get(), ASTNodeId::NODE_BLOCK_BLOCK_ID);
        if (HEDLEY_UNLIKELY(nodeBlockType == NodeBlockType::BLOCK || blockId.isError())) {
            tokenReader.pop();
            return blockId;
        }
        tokenReader.push();
        ASTNode blockStateLeftBracket = nodeBlockStateLeftBracket->getASTNode(tokenReader, cpack);
        tokenReader.restore();
        if (HEDLEY_LIKELY(blockStateLeftBracket.isError())) {
            return ASTNode::andNode(this, {blockId}, tokenReader.collect(),
                                    nullptr, ASTNodeId::NODE_BLOCK_BLOCK_AND_BLOCK_STATE);
        }
        size_t strHash = std::hash<std::string_view>{}(blockId.tokens.toString());
        std::shared_ptr<NamespaceId> currentBlock = nullptr;
        for (const auto &item: *blockIds) {
            if (HEDLEY_UNLIKELY(item->fastMatch(strHash) || item->getIdWithNamespace()->fastMatch(strHash))) {
                currentBlock = item;
                break;
            }
        }
        auto nodeBlockState = currentBlock == nullptr ? BlockId::getNodeAllBlockState() : std::static_pointer_cast<BlockId>(currentBlock)->getNode().get();
        auto astNodeBlockState = getByChildNode(tokenReader, cpack, nodeBlockState, ASTNodeId::NODE_BLOCK_BLOCK_STATE);
        return ASTNode::andNode(this, {blockId, astNodeBlockState}, tokenReader.collect(),
                                nullptr, ASTNodeId::NODE_BLOCK_BLOCK_AND_BLOCK_STATE);
    }

    std::optional<std::string> NodeBlock::collectDescription(const ASTNode *node, size_t index) const {
        if (HEDLEY_LIKELY(node->id == ASTNodeId::NODE_BLOCK_BLOCK_ID)) {
            return "方块ID";
        } else if (HEDLEY_LIKELY(node->id == ASTNodeId::NODE_BLOCK_BLOCK_STATE)) {
            return "方块状态";
        } else {
            return std::nullopt;
        }
    }

    bool NodeBlock::collectSuggestions(const ASTNode *astNode,
                                       size_t index,
                                       std::vector<Suggestions> &suggestions) const {
        if (HEDLEY_LIKELY(astNode->id == ASTNodeId::NODE_BLOCK_BLOCK_AND_BLOCK_STATE && !astNode->isError() &&
                          astNode->childNodes.size() == 1 && index == astNode->tokens.getEndIndex())) {
            suggestions.push_back(Suggestions::singleSuggestion({index, index, false, nodeBlockStateLeftBracket->normalId}));
        }
        return false;
    }

    void NodeBlock::collectStructure(const ASTNode *astNode, StructureBuilder &structure, bool isMustHave) const {
        structure.append(isMustHave, "方块ID");
        if (HEDLEY_LIKELY(nodeBlockType == NodeBlockType::BLOCK_WITH_BLOCK_STATE)) {
            structure.append(false, "方块状态");
        }
    }

    namespace NodeBlockType {

        CODEC_ENUM(NodeBlockType, uint8_t)

    }// namespace NodeBlockType

    CODEC_NODE(NodeBlock, nodeBlockType)

}// namespace CHelper::Node