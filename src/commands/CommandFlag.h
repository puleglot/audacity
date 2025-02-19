//
//  CommandFlag.h
//  Audacity
//
//  Created by Paul Licameli on 11/22/16.
//
//

#ifndef __AUDACITY_COMMAND_FLAG__
#define __AUDACITY_COMMAND_FLAG__

// Flags used in command handling.

#include <bitset>
#include <functional>
#include <utility>
#include <wx/string.h>

class AudacityProject;

// Increase the template parameter as needed to allow more flags
constexpr size_t NCommandFlags = 64;
static_assert(
   NCommandFlags <= 8 * sizeof( unsigned long long ),
   "NoFlagsSpecified may have incorrect value"
);

// Type to specify conditions for enabling of a menu item
using CommandFlag = std::bitset<NCommandFlags>;

// Special constant values
constexpr CommandFlag
   AlwaysEnabledFlag{},      // all zeroes
   NoFlagsSpecified{ ~0ULL }; // all ones

struct CommandFlagOptions{
   // Supplied the translated name of the command, returns a translated
   // error message
   using MessageFormatter = std::function< wxString( const wxString& ) >;

   CommandFlagOptions() = default;
   CommandFlagOptions(
      const MessageFormatter &message_,
      const wxString &helpPage_ = {},
      const wxString &title_ = {}
   ) : message{ message_ }, helpPage{ helpPage_ }, title{ title_ }
   {}

   CommandFlagOptions && QuickTest() &&
   { quickTest = true; return std::move( *this ); }
   CommandFlagOptions && DisableDefaultMessage() &&
   { enableDefaultMessage = false; return std::move( *this ); }
   CommandFlagOptions && Priority( unsigned priority_ ) &&
   { priority = priority_; return std::move( *this ); }

   // null, or else computes non-default message for the dialog box when the
   // condition is not satisfied for the selected command
   MessageFormatter message;

   // Title and help page are used only if a message function is given
   wxString helpPage;

   // Empty, or non-default title for the dialog box when the
   // condition is not satisfied for the selected command
   // This string must be given UN-translated.
   wxString title;

   // Conditions with higher "priority" are preferred over others in choosing
   // the help message
   unsigned priority = 0;

   // If false, and no other condition with a message is unsatisfied, then
   // display no dialog box at all when this condition is not satisfied
   bool enableDefaultMessage = true;

   // If true, assume this is a cheap test to be done always.  If false, the
   // test may be skipped and the condition assumed to be unchanged since the
   // last more comprehensive testing
   bool quickTest = false;
};

// Construct one statically to register (and reserve) a bit position in the set
// an associate it with a test function; those with quickTest = true are cheap
// to compute and always checked
class ReservedCommandFlag : public CommandFlag
{
public:
   using Predicate = std::function< bool( const AudacityProject& ) >;
   ReservedCommandFlag( const Predicate &predicate,
      const CommandFlagOptions &options = {} );
};

// Widely used command flags, but this list need not be exhaustive.  It may be
// extended, with special purpose flags of limited use, by constucting static
// ReservedCommandFlag values

// To describe auto-selection, stop-if-paused, etc.:
// A structure describing a set of conditions, another set that might be
// made true given the first, and the function that may make them true.
// If a menu item requires the second set, while the first set is true,
// then the enabler will be invoked (unless the menu item is constructed with
// the useStrictFlags option, or the applicability test first returns false).
// The item's full set of required flags is passed to the function.

// Computation of the flags is delayed inside a function -- because often you
// need to name a statically allocated CommandFlag, or a bitwise OR of some,
// while they may not have been initialized yet, during static initialization.
struct MenuItemEnabler {
   using Flags = std::function< CommandFlag() >;
   using Test = std::function< bool( const AudacityProject& ) >;
   using Action = std::function< void( AudacityProject&, CommandFlag ) >;

   const Flags actualFlags;
   const Flags possibleFlags;
   Test applicable;
   Action tryEnable;
};

// Typically this is statically constructed:
struct RegisteredMenuItemEnabler{
   RegisteredMenuItemEnabler( const MenuItemEnabler &enabler );
};

#endif
