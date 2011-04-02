struct CoverSearchResult {

%TypeHeaderCode
#include "covers/albumcoverfetcher.h"
%End

%Docstring
Represents a single result of some album's cover search request.

It contains an URL that leads to a found cover plus it's description (usually
the "artist - album" string).
%End

  QString description;
%Docstring
Description of this result.

We suggest using the "artist - album" format.
%End

  QString image_url;
%Docstring
An URL of a cover image described by this CoverSearchResult
%End
};

typedef QList<CoverSearchResult> CoverSearchResults;
