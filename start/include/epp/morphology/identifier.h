#ifndef EPP_INCLUDE_EPP_MORPHOLOGY_IDENTIFIER_H_
#define EPP_INCLUDE_EPP_MORPHOLOGY_IDENTIFIER_H_

#include <optional>
#include <string>
#include <string_view>

namespace epp::morphology {

// 識別子の形態論上の品詞を表す。
enum class PartOfSpeech {
  kUnknown,
  kNeutral,
  kNoun,
  kAdjective,
  kAdverb,
  kInfinitive,
  kImperative
};

// 識別子が担う役割を表す。
enum class IdentifierRole {
  kInvalid,
  kValueName,
  kTypeName,
  kFunctionName,
  kDirectCallName,
};

// 識別子を形態論的に解析した結果を保持する。
struct IdentifierInfo {
  std::u32string normalized_identifier;
  std::u32string root;
  PartOfSpeech part_of_speech;
  bool has_plural_ending;
  bool has_accusative_ending;
};

// 正規化済み識別子から、語根、品詞、語尾の情報を取り出す。
[[nodiscard]] IdentifierInfo AnalyzeIdentifier(
  std::u32string_view normalized_identifier
);

// 命令形の直接呼び出しから、対応する不定詞の関数名を返す。
// 命令形として使用できなければstd::nulloptを返す。
[[nodiscard]] std::optional<std::u32string>
GetFunctionIdentifierForDirectCall(std::u32string_view direct_call_identifier);

// 形態論解析結果から、識別子の役割を判定する。
// 判定不可の場合にはkInvalidを返す。
[[nodiscard]] IdentifierRole ClassifyIdentifierRole(
  const IdentifierInfo& identifier_info
) noexcept;

} // namespace epp::morphology

#endif // EPP_INCLUDE_EPP_MORPHOLOGY_IDENTIFIER_H_
