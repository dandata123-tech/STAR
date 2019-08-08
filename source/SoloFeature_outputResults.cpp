#include "SoloFeature.h"
#include "streamFuns.h"
#include "TimeFunctions.h"
#include "serviceFuns.cpp"
#include "SequenceFuns.h"
#include "ErrorWarning.h"

void SoloFeature::outputResults(bool cellFilterYes)
{    
    string outputPrefix1;
    if (cellFilterYes) {
        outputPrefix1 = outputPrefix + "/filtered/";
    } else {
        outputPrefix1 = outputPrefix + "/raw/";
    };
    
    if (mkdir(outputPrefix1.c_str(),P.runDirPerm)!=0 && errno!=EEXIST) {//create directory
        exitWithError("EXITING because of fatal OUTPUT FILE error: could not create Solo output directory" + outputPrefix1 + 
                      "\nSOLUTION: check the path and permisssions\n",
                       std::cerr, P.inOut->logMain, EXIT_CODE_PARAMETER, P);
    };
            
    if (featureType==0 || featureType==1 ) {//this onlys need to be done once
        //output genes
        ofstream &geneStr=ofstrOpen(outputPrefix1+pSolo.outFileNames[1],ERROR_OUT, P);
        for (uint32 ii=0; ii<Trans.nGe; ii++)
            geneStr << Trans.geID[ii] <<"\t"<< (Trans.geName[ii].empty() ? Trans.geID[ii] : Trans.geName[ii]) << '\n';
        geneStr.close();
    };

    //output CBs
    ofstream &cbStr=ofstrOpen(outputPrefix1+pSolo.outFileNames[2],ERROR_OUT, P);
    
    if (cellFilterYes) {
        for (uint32 icb=0; icb<nCB; icb++) {
            if (cellFilterVec[icb])
                cbStr << pSolo.cbWLstr[indCB[icb]] <<'\n';
        };
    } else {
        for (uint64 ii=0; ii<pSolo.cbWLsize; ii++)
             cbStr << pSolo.cbWLstr[ii] <<'\n';
    };
    cbStr.flush();

    //output counting matrix
    string matrixFileName=outputPrefix1+pSolo.outFileNames[3];
    ofstream &countMatrixStream=ofstrOpen(matrixFileName,ERROR_OUT, P);

    //header
    uint32 featureN=0;
    if (featureType==0 || featureType==2) {//genes
        featureN=Trans.nGe;
    } else if (featureType==1) {//sj
        featureN=P.sjAll[0].size();
    };
    countMatrixStream <<"%%MatrixMarket matrix coordinate integer general\n%\n";
    countMatrixStream << featureN <<' '<< pSolo.cbWLsize <<' '<< nCellGeneEntries << '\n';

    uint32  cbInd1=0;
    for (uint32 icb=0; icb<nCB; icb++) {
        uint32 *rCBpp=rCBp[icb];
        if (cellFilterYes) {
            if (cellFilterVec[icb]) {
                ++cbInd1;
            } else {
                continue;
            };
        } else {
            cbInd1=indCB[icb]+1;
        };
        for (uint32 ig=0; ig<nGperCB[icb]; ig++) {
            
            //feature index, CB index
            countMatrixStream << rCBpp[0]+1  <<' '<< cbInd1;
            
            //count(s)
            uint32 count1[3] = {1,1,1};
            if (rCBpp[1]>1) {//3 counts recorded
                count1[0] = rCBpp[1];
                count1[1] = rCBpp[2];
                count1[2] = rCBpp[3];
                rCBpp += 4;
            } else {//1 recorded
                rCBpp += 2;
            };
            for (uint32 ii=0; ii<pSolo.umiDedupColumns.size(); ii++)
                countMatrixStream <<' '<< count1[pSolo.umiDedupColumns[ii]];
            
            countMatrixStream << '\n';
        };
    };
    countMatrixStream.flush();
};
