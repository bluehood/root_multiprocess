#include "TNote.h"

std::string to_string(TNote::ECode c)
{
   switch (c) {
      case TNote::kMessage:
         return "message";
      case TNote::kError:
         return "error";
      case TNote::kFatalError:
         return "fatal error";
      case TNote::kExecClass:
         return "execClass";
      case TNote::kExecMacro:
         return "execMacro";
      case TNote::kShutdownOrder:
         return "shutdownOrder";
      case TNote::kClassResult:
         return "classResult";
      case TNote::kMacroResult:
         return "macroResult";
      case TNote::kShutdownNotice:
         return "shutdownNotice";
      default:
         return "unkown code";
   }
}


TNote::TNote()
{
   code = kMessage;
   str = "";
   obj = nullptr;
}
