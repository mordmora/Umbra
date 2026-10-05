#include "umbra/source/SourceManager.h"
#include "umbra/preprocessor/Preprocessor.h"
#include "umbra/lexer/Lexer.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>

namespace {

class TempTree {
public:
    TempTree() {
        static std::atomic<unsigned> next{0};
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        root = std::filesystem::temp_directory_path() /
               ("umbra-source-test-" + std::to_string(stamp) + "-" +
                std::to_string(next++));
        std::filesystem::create_directories(root);
    }
    ~TempTree() { std::filesystem::remove_all(root); }

    std::filesystem::path write(const std::filesystem::path& relative,
                                const std::string& bytes) const {
        const auto path = root / relative;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream stream(path, std::ios::binary);
        stream.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        return path;
    }

    std::filesystem::path root;
};

TEST(SourceManagerTest, ResolvesImportsAndPreservesOriginalOffsets) {
    TempTree tree;
    const auto entry = tree.write("main.umbra", "func start() -> void {}\n");
    tree.write("lib/math.umbra", "\xEF\xBB\xBF\xC3\xA9\nx");

    umbra::SourceManager sources;
    const auto mainResult = sources.loadFile(entry);
    ASSERT_TRUE(std::holds_alternative<umbra::FileId>(mainResult));
    const auto mainId = std::get<umbra::FileId>(mainResult);

    const auto importResult = sources.loadFile("lib/math.umbra", mainId);
    ASSERT_TRUE(std::holds_alternative<umbra::FileId>(importResult));
    const auto importedId = std::get<umbra::FileId>(importResult);
    const auto& imported = sources.file(importedId);
    EXPECT_EQ(imported.text(), "\xC3\xA9\nx");
    EXPECT_EQ(imported.originalOffset(0), 3u);
    EXPECT_EQ(imported.originalOffset(3), 6u);
    EXPECT_EQ(imported.lineColumn(2).line, 1u);
    EXPECT_EQ(imported.lineColumn(2).column, 2u);
    EXPECT_EQ(imported.lineColumn(3).line, 2u);
    EXPECT_EQ(imported.lineColumn(3).column, 1u);

    const auto again = sources.loadFile("./lib/../lib/math.umbra", mainId);
    ASSERT_TRUE(std::holds_alternative<umbra::FileId>(again));
    EXPECT_EQ(std::get<umbra::FileId>(again), importedId);
    EXPECT_EQ(sources.fileCount(), 2u);
}

TEST(SourceManagerTest, RejectsInvalidUtf8WithoutRegisteringFile) {
    TempTree tree;
    const auto invalid = tree.write("invalid.umbra", "\xC3\x28");
    umbra::SourceManager sources;

    const auto result = sources.loadFile(invalid);
    ASSERT_TRUE(std::holds_alternative<umbra::SourceError>(result));
    EXPECT_EQ(std::get<umbra::SourceError>(result).byteOffset, 0u);
    EXPECT_EQ(sources.fileCount(), 0u);
}

TEST(SourceManagerTest, ReportsMissingPathsAndAcceptsEmptyFiles) {
    TempTree tree;
    umbra::SourceManager sources;
    EXPECT_TRUE(std::holds_alternative<umbra::SourceError>(
        sources.loadFile(tree.root / "missing.umbra")));

    const auto empty = tree.write("empty.umbra", "");
    const auto result = sources.loadFile(empty);
    ASSERT_TRUE(std::holds_alternative<umbra::FileId>(result));
    EXPECT_TRUE(sources.file(std::get<umbra::FileId>(result)).text().empty());
}

TEST(SourceManagerTest, PreprocessorReadsBomAndRelativeImport) {
    TempTree tree;
    const auto entry = tree.write("main.umbra",
        "use \"lib/math.umbra\"\nfunc start() -> int { return answer() }\n");
    tree.write("lib/math.umbra",
        "\xEF\xBB\xBF" "func answer() -> int { return 42 }\n");

    const umbra::Preprocessor preprocessor(entry.string());
    const auto content = preprocessor.getProcessedContent();
    EXPECT_EQ(content.find("\xEF\xBB\xBF"), std::string::npos);
    EXPECT_NE(content.find("func answer()"), std::string::npos);
    EXPECT_NE(content.find("func start()"), std::string::npos);
}

TEST(SourceManagerTest, LexerRetainsFileAndByteRanges) {
    TempTree tree;
    const auto path = tree.write("source.umbra",
        "\xEF\xBB\xBF" "int x = 1\n\"\xC3\xA9\"\n");
    umbra::SourceManager sources;
    const auto loaded = sources.loadFile(path);
    ASSERT_TRUE(std::holds_alternative<umbra::FileId>(loaded));
    const auto id = std::get<umbra::FileId>(loaded);

    umbra::Lexer lexer(sources.file(id));
    const auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 8u);
    EXPECT_EQ(tokens[0].span.file, id);
    EXPECT_EQ(tokens[0].span.begin, 0u);
    EXPECT_EQ(tokens[0].span.end, 3u);
    EXPECT_EQ(tokens[4].type, umbra::TokenType::TOK_NEWLINE);
    EXPECT_EQ(tokens[5].lexeme, "\xC3\xA9");
    EXPECT_EQ(tokens[5].span.begin, 10u);
    EXPECT_EQ(tokens[5].span.end, 14u);
    EXPECT_EQ(tokens[5].line, 2);
    EXPECT_EQ(tokens[5].column, 1);
    EXPECT_EQ(tokens.back().span.begin, sources.file(id).text().size());
    EXPECT_EQ(tokens.back().span.end, sources.file(id).text().size());
    EXPECT_EQ(sources.file(id).originalOffset(tokens[5].span.begin), 13u);
}

TEST(SourceManagerTest, LexerAcceptsEmptySourceFile) {
    TempTree tree;
    umbra::SourceManager sources;
    const auto loaded = sources.loadFile(tree.write("empty.umbra", ""));
    ASSERT_TRUE(std::holds_alternative<umbra::FileId>(loaded));
    const auto id = std::get<umbra::FileId>(loaded);
    umbra::Lexer lexer(sources.file(id));
    const auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].span.file, id);
    EXPECT_EQ(tokens[0].span.begin, 0u);
    EXPECT_EQ(tokens[0].span.end, 0u);
}

} // namespace
