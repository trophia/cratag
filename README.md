### cratag

Code for pre-processing New Zealand rock lobster tag release-recapture data
for input into stock assessments. Does things such as:

- linking releases to the next recapture
- converting carapace length to tail width
- checking the consistency of releases and recaptures (i.e. same sexes, not very large negative increments)

### Background

This code was originally written circa 2000-2002. A binary executable based on this code (or perhaps
some slight modification of it?) has been used for assessments since. In 2015, some archeology was
done to recover the original source code.  The code was condensed into a single file and made compilable using
the C++ standard library (rather than third party libraries). 

### Status

Not all aspects have been fully reimplemented (there are `#warning`s in the code for some known issues)
and as it stands this code will not produce the same outputs as the binary. It may not be worth 
trying to fully reimplement this code. Instead, a new implementation, using modern coding standards and potentially
a different language, could be more worthwhile and is unlikely to be too onerous. In that case, this code provides
some documentation of what pre-processing of tag data has been performed in the past.
