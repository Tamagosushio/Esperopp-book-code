#include "epp/morphology/identifier.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace epp::morphology {

namespace {

// 識別子に使用できる英字、エスペラント文字か判定する。
bool IsIdentifierLetter(char32_t scalar) noexcept {
  constexpr std::u32string_view kIdentifierLetters{
    U"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    U"abcdefghijklmnopqrstuvwxyz"
    U"ĈĉĜĝĤĥĴĵŜŝŬŭ"
  };
  return kIdentifierLetters.find(scalar) != std::u32string_view::npos;
}

// ASCII大文字を小文字に変換する。
char32_t ToAsciiLower(char32_t scalar) noexcept {
  if (U'A' <= scalar && scalar <= U'Z') {
    return scalar + (U'a' - U'A');
  }
  return scalar;
}

// 一文字の語尾から品詞を判定する。
PartOfSpeech ClassifyPartOfSpeech(char32_t ending) noexcept {
  switch (ToAsciiLower(ending)) {
    case U'o': return PartOfSpeech::kNoun;
    case U'a': return PartOfSpeech::kAdjective;
    case U'e': return PartOfSpeech::kAdverb;
    case U'i': return PartOfSpeech::kInfinitive;
    case U'u': return PartOfSpeech::kImperative;
    default:
      return PartOfSpeech::kUnknown;
  }
}

} // namespace

IdentifierInfo AnalyzeIdentifier(std::u32string_view normalized_identifier) {
  IdentifierInfo identifier_info{
    std::u32string(normalized_identifier),
    U"",
    PartOfSpeech::kUnknown,
    false,
    false
  };
  // 一字の識別子は、語尾無しとして扱う。
  if (normalized_identifier.size() == 1 && IsIdentifierLetter(normalized_identifier.front())) {
    identifier_info.root = normalized_identifier;
    identifier_info.part_of_speech = PartOfSpeech::kNeutral;
    return identifier_info;
  }

  std::size_t ending_offset = normalized_identifier.size();
  // 対格語尾-nがあれば記録し、品詞の解析対象から取り除く。
  if (ending_offset > 0 && ToAsciiLower(normalized_identifier[ending_offset - 1]) == U'n') {
    identifier_info.has_accusative_ending = true;
    --ending_offset;
  }
  // 複数語尾-jがあれば記録し、品詞の解析対象から取り除く。
  if (ending_offset > 0 && ToAsciiLower(normalized_identifier[ending_offset - 1]) == U'j') {
    identifier_info.has_plural_ending = true;
    --ending_offset;
  }
  if (ending_offset == 0) {
    return identifier_info;
  }

  const PartOfSpeech part_of_speech = ClassifyPartOfSpeech(normalized_identifier[ending_offset - 1]);
  // 品詞語尾がない場合と、品詞語尾を除くと語根が空になる場合は、kUnknownのまま返す。
  if (part_of_speech == PartOfSpeech::kUnknown || ending_offset == 1) {
    return identifier_info;
  }
  identifier_info.root = normalized_identifier.substr(0, ending_offset - 1);
  identifier_info.part_of_speech = part_of_speech;

  return identifier_info;
}

std::optional<std::u32string> GetFunctionIdentifierForDirectCall(
  std::u32string_view direct_call_identifier
) {
  const IdentifierInfo identifier_info = AnalyzeIdentifier(direct_call_identifier);

  if (ClassifyIdentifierRole(identifier_info) != IdentifierRole::kDirectCallName) {
    return std::nullopt;
  }

  std::u32string function_identifier(direct_call_identifier);

  // 命令形語尾-uを不定詞語尾-iへ置き換える。
  function_identifier.back() = function_identifier.back() == U'U' ? U'I' : U'i';

  return function_identifier;
}

IdentifierRole ClassifyIdentifierRole(
  const IdentifierInfo& identifier_info
) noexcept {
  if (identifier_info.has_plural_ending || identifier_info.has_accusative_ending) {
    return IdentifierRole::kInvalid;
  }

  switch (identifier_info.part_of_speech) {
    case PartOfSpeech::kNeutral:
    case PartOfSpeech::kNoun:
      return IdentifierRole::kValueName;
    case PartOfSpeech::kAdjective:
      return IdentifierRole::kTypeName;
    case PartOfSpeech::kInfinitive:
      return IdentifierRole::kFunctionName;
    case PartOfSpeech::kImperative:
      return IdentifierRole::kDirectCallName;
    case PartOfSpeech::kUnknown:
    case PartOfSpeech::kAdverb:
      return IdentifierRole::kInvalid;
  }
  return IdentifierRole::kInvalid;
}

} // namespace epp::morphology
