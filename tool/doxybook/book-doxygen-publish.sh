#!/usr/bin/env bash

## @file   book-doxygen-publish.sh
#  @brief  The PDF generator for a Doxygen-based developer manual
#  @date   May-13-2018
#  @author Geunsik Lim <geunsik.lim@samsung.com>

################# Do not modify the below statements ##################################

##
#  @brief check if a package is installed
#  @param
#   arg1: package name
function check_dep_cmd() {
    echo "Checking for $1..."
    which "$1" 2>/dev/null || {
      echo "Please install $1."
      exit 1
    }
}

## @brief Display debug message
function display(){
    echo -e $1 $2
}

## @brief Generate original book with doxygen tags
function generate_original(){
    display "[DEBUG] current folder:" $(pwd)
    doxygen $REFERENCE_REPOSITORY/tool/doxybook/Doxyfile.prj
    cd latex 
    make  -j`nproc`
    mv refman.pdf book-prj.pdf 
}

## @brief Generate cover book
function generate_cover(){
    display "[DEBUG] current folder:" $(pwd)
    rm -f $REFERENCE_REPOSITORY/tool/doxybook/.~lock.book-cover-design.pdf
    wget https://github.com/nnstreamer/TAOS-CI/raw/main/ci/doxybook/book-cover-design.odt -P $REFERENCE_REPOSITORY/tool/doxybook/
    unoconv -f pdf --export=ExportFormFields=false $REFERENCE_REPOSITORY/tool/doxybook/book-cover-design.odt
    mv $REFERENCE_REPOSITORY/tool/doxybook/book-cover-design.pdf ./
    string_time="\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n                         $(date)"
    echo -e  "$string_time" > book-cover-date.txt
    libreoffice --convert-to "pdf" book-cover-date.txt
    if [[ ! -f "book-cover-date.pdf" ]]; then
      echo -e  "[DEBUG] Oooops. generate_cover function."
      echo -e  "[DEBUG] Please replace 'libreoffice' with 'sudo libreoffice' to fix this issue."
      exit 99
    fi
    pdftk book-cover-design.pdf stamp book-cover-date.pdf output book-cover-stamped.pdf
}

## @brief Generate final book
function generate_final(){
    display "[DEBUG] current folder:" $(pwd)
    pdfunite book-cover-stamped.pdf book-prj.pdf book.pdf
    display "[DEBUG] PDF book file is generated in" $(pwd)
}

## @brief Clean unnecessary files
function generate_clean(){
    mv $REFERENCE_REPOSITORY/latex/book.pdf $REFERENCE_REPOSITORY
    display "[DEBUG] Clean doxygen folder"
    rm -rf $REFERENCE_REPOSITORY/latex
    rm -rf $REFERENCE_REPOSITORY/html
}

## @brief Main function
function main(){
    # fetch a project path from command
    display "[DEBUG] current folder:" $1
    prj_path="`dirname \"$1\"`/../.."

    # go to project path
    cd $prj_path
    display "[DEBUG] value of prj_path:" $prj_path
    display "[DEBUG] current folder:" $(pwd)

    # let's go to source folder
    # then, run entire procedure to generate doxygen-based pdf book
    pushd $REFERENCE_REPOSITORY
    display "[DEBUG] current folder:" $(pwd)
    generate_original
    generate_cover
    generate_final
    generate_clean
    popd

    echo -e "The Doxygen-based publication procedure is completed."
}

## @dependency: doxygen, make, unoconv, libreoffice, pdftk, pdfunite
check_dep_cmd doxygen
check_dep_cmd make
check_dep_cmd unoconv
check_dep_cmd libreoffice
check_dep_cmd pdftk
check_dep_cmd pdfunite

# Start
# Note that the you must keep the folder structure to generate the doxygen-based pdf book as follows.
# Run this script from root path of the project
REFERENCE_REPOSITORY=$(pwd)
echo $REFERENCE_REPOSITORY
main $0

