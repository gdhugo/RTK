#!/usr/bin/env bash

# Updates the documentation strings in the JSON files from the ITK Doxygen.
#
# Configuration for directories need to be manually done in the
# config_vars.sh file. The RTK Doxygen XML will automatically be
# downloaded if needed.
#
# Usage: JSONDocUpdate.sh Code/BasicFilters/json/SomeFilter.json
#

die() {
    echo "$@" 1>&2
    exit 1
}

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Load configuration variable from file
. ${DIR}/config_vars.sh || die 'Unable to find local \"config_vars.sh\" configuration file.'

# Download the nightly Doxygen XML if needed
[ -e ${RTK_XML}/itkImage_8h.xml ] ||
      ( cd ${RTK_XML} &&
        echo "Downloading ITK Doxygen XML..." &&
        curl -O http://public.kitware.com/pub/itk/NightlyDoxygen/InsightDoxygenDocXml.tar.gz &&
        cd .. &&
        tar -zxf xml/InsightDoxygenDocXml.tar.gz ) ||
      die 'Unable to get ITK Doxygen XML'

# Does the work of extracting the string from the XML, and putting them into the JSON
${GROOVEY_EXECUTABLE} "${DIR}/GenerateDocumentation.groovy" "$1"  ${RTK_XML} ||
  die 'Error running GenerateDocumentation.groovy'


# Perform standard formatting of the JSON file
${PYTHON_EXECUTABLE} "${SIMPLEITK}/Utilities/JSONBeautify.py" $1 ||
  die 'Error Beautifying json file $1'
