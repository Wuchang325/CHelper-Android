//
// Created by Yancey on 2024/2/24.
//

#include "Core.h"

#include "parser/Parser.h"

namespace CHelper {

    Core::Core(std::unique_ptr<CPack> cpack, ASTNode astNode)
        : cpack(std::move(cpack)),
          astNode(std::move(astNode)) {}

    Core *Core::create(const std::function<std::unique_ptr<CPack>()> &getCPack) {
        try {
#if CHelperWeb != true
            std::chrono::high_resolution_clock::time_point start, end;
            start = std::chrono::high_resolution_clock::now();
#endif
            std::unique_ptr<CPack> cPack = getCPack();
#if CHelperWeb != true
            end = std::chrono::high_resolution_clock::now();
            CHELPER_INFO(ColorStringBuilder()
                                 .green("CPack load successfully (")
                                 .purple(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + "ms")
                                 .green(")")
                                 .build());
#endif
            ASTNode astNode = Parser::parse("", cPack.get());
            return new Core(std::move(cPack), std::move(astNode));
        } catch (const std::exception &e) {
            CHELPER_ERROR("CPack load failed");
            CHelper::Profile::printAndClear(e);
            return nullptr;
        }
    }

#if CHelperSupportJson == true
    Core *Core::createByDirectory(const std::filesystem::path &cpackPath) {
        return create([&cpackPath]() {
            return CPack::createByDirectory(cpackPath);
        });
    }

    Core *Core::createByJson(const std::filesystem::path &cpackPath) {
        return create([&cpackPath]() {
            return CPack::createByJson(JsonUtil::getJsonFromFile(cpackPath));
        });
    }

    Core *Core::createByBson(const std::filesystem::path &cpackPath) {
        return create([&cpackPath]() {
            return CPack::createByJson(JsonUtil::getBsonFromFile(cpackPath));
        });
    }
#endif

#if CHelperWeb != true
    Core *Core::createByBinary(const std::filesystem::path &cpackPath) {
        return create([&cpackPath]() {
            // 检查文件名后缀
            std::string fileType = ".cpack";
            std::string cpackPathStr = cpackPath.string();
            if (HEDLEY_UNLIKELY(cpackPathStr.size() < fileType.size() || cpackPathStr.substr(cpackPathStr.length() - fileType.size()) != fileType)) {
                throw std::runtime_error("error file type");
            }
            // 打开文件
            std::ifstream is(cpackPath, std::ios::binary);
            if (HEDLEY_UNLIKELY(!is.is_open())) {
                throw std::runtime_error("fail to read file: " + cpackPath.string());
            }
            // 读取文件
            BinaryReader binaryReader(true, is);
            std::unique_ptr<CPack> result = CPack::createByBinary(binaryReader);
            // 检查文件是否读完
            if (HEDLEY_UNLIKELY(!is.eof())) {
                char ch;
                is.read(&ch, 1);
                if (HEDLEY_UNLIKELY(is.gcount() > 0)) {
                    throw std::runtime_error("file is not read completed: " + cpackPathStr);
                }
            }
            // 关闭文件
            is.close();
            return std::move(result);
        });
    }
#endif

    void Core::onTextChanged(const std::string &content, size_t index0) {
        if (HEDLEY_LIKELY(input != content)) {
            input = content;
            astNode = Parser::parse(input, cpack.get());
            suggestions = nullptr;
        }
        onSelectionChanged(index0);
    }

    void Core::onSelectionChanged(size_t index0) {
        if (HEDLEY_LIKELY(index != index0)) {
            index = index0;
            suggestions = nullptr;
        }
    }

    [[nodiscard]] const CPack *Core::getCPack() const {
        return cpack.get();
    }

    [[nodiscard]] const ASTNode *Core::getAstNode() const {
        return &astNode;
    }

    [[nodiscard]] std::string Core::getDescription() const {
        return astNode.getDescription(index);
    }

    [[nodiscard]] std::vector<std::shared_ptr<ErrorReason>> Core::getErrorReasons() const {
        return astNode.getErrorReasons();
    }

    std::vector<Suggestion> *Core::getSuggestions() {
        if (HEDLEY_LIKELY(suggestions == nullptr)) {
            suggestions = std::make_shared<std::vector<Suggestion>>(astNode.getSuggestions(index));
        }
        return suggestions.get();
    }

    [[nodiscard]] std::string Core::getStructure() const {
        return astNode.getStructure();
    }

    [[nodiscard]] [[maybe_unused]] std::string Core::getColors() const {
        return astNode.getColors();
    }

    std::optional<std::string> Core::onSuggestionClick(size_t which) {
        if (HEDLEY_UNLIKELY(suggestions == nullptr || which >= suggestions->size())) {
            return std::nullopt;
        }
        return suggestions->at(which).apply(this, astNode.tokens.toString());
    }

    std::string Core::old2new(const Old2New::BlockFixData &blockFixData, const std::string &old) {
        return Old2New::old2new(blockFixData, old);
    }

}// namespace CHelper