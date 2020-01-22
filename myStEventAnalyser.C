#include <map>
//#include <TROOT.h>
#include <vector>
#include <iostream>

/*
	struct hitContainer {
		float adc;
		float hitx;
		float hity;
		float hitz;
	} ;
*/
using namespace std;
void myStEventAnalyser(const int numberOfEvents, const char * file, const int eventToSelect = -1, const char * eventsFile = "") {
	cout << file << " to be analyzed" << endl;
	cout << eventToSelect << " to be selected" << endl;
	cout << eventsFile << " will be checked for events to work with" << endl;

//	vector<int> eventVec; // vector for events to display
/*
	FILE * inEvents;
	inEvents = fopen(eventsFile, "r");
	while (!feof (inEvents)) {
		char eventFromFile[100];
		if ( fgets (eventFromFile, 100 , inEvents) == NULL ) break;
		fputs (eventFromFile, stdout);
		cout << eventFromFile << endl;
//		eventVec.push_back(atoi(eventFromFile));
	}
	fclose (inEvents);
*/
//	hitContainer tempHit;

//	map<int, vector<hitContainer> > myHits;

	FILE * dataOut;
	FILE * hitMap;

	char dataOutName[100];
	char hitMapName[100];

//	const char * file = "st_cosmic_adc_19053068_raw_2000015.event.root";
	gROOT->LoadMacro("$STAR/StRoot/StMuDSTMaker/COMMON/macros/loadSharedLibraries.C");
	loadSharedLibraries();

	gROOT->LoadMacro("bfc.C");
	TString Chain("StEvent, nodefault, mysql, in");
//	TString Chain("StEvent, nodefault, mysql, in, quiet");
	bfc(0, Chain, file); // This will make chain

//	LOOP OVER EVENTS
	int printCounter = 0;

	for (int event = 0; event < numberOfEvents; event++) {
		int iMake = chain->MakeEvent();
		if (iMake % 10 == kStEOF || iMake % 10==kStFatal) {
			cout << "SOMETHING BAD HAPPENED TO THE CHAIN!" << endl;
			break;
		}

		StEvent * pEvent = (StEvent*) chain->GetInputDS("StEvent");
		if (!pEvent) {
			cout << "    - - - - - - - !!! - - - - - - - There is no StEvent?!" << endl;
			continue;
		}
		StSPtrVecTrackNode & myTrackNode = pEvent->trackNodes();

		if (printCounter % 1000 == 0) cout << "Working on event " << printCounter << "(" << event << ")" << " -=- * * * Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << " * * * -=-" << endl;
		printCounter++;

		int myNTracks = myTrackNode.size();

//		THIS IS TO SELECT EVENTS WITH SOME PRESET CUTS
#if 0
		StPrimaryVertex * pv = pEvent->primaryVertex();
//		double pvz = pv->position().z();
//		if (fabs(pvz) > 5) {
//			event--;
//			continue;
//		}
		if (myNTracks < 500) {
			event--;
			continue;
		}
# endif

		if (eventToSelect != -1 && pEvent->id() != eventToSelect) {
				event--;
			       	continue; // event selector condition
			}

		StTpcHitCollection* TpcHitCollection = pEvent->tpcHitCollection();
		if (!TpcHitCollection) {
			cout << "No TPC Hit Collection in this event, skipping to the next one." << endl;
			continue;
		}

		cout << " -=- * * * Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << " * * * -=-" << endl;
		sprintf(dataOutName, "data_%d_%d.csv", pEvent->runId(),  pEvent->id());
		sprintf(hitMapName, "hitMap_%d_%d.csv", pEvent->runId(),  pEvent->id());


		dataOut = fopen(dataOutName, "w");
		hitMap = fopen(hitMapName, "w");

//		ADDING HEADER TO THE OUTPUT FILES
		fprintf(dataOut, "xi, yi, zi, xf, yf, zf, px, py, pz, r, g, b,  c, curv, pt\n");
		fprintf(hitMap, "sector, row, padMin, padMax, adc, x, y\n");

//		All hits and sector loop below ==============================
		for (int sec = 0; sec < 24; sec++) {
			StTpcSectorHitCollection* sectorCollection = TpcHitCollection->sector(sec);

			unsigned int numberOfPadrows = sectorCollection->numberOfPadrows();
			unsigned long NoHits = 0;
			unsigned long NoBadHits = 0;
			bool doOnce = false;
			for (int row = 0; row < numberOfPadrows; row++) {
				StTpcPadrowHitCollection *rowCollection = sectorCollection->padrow(row);
				if (rowCollection) {
					StSPtrVecTpcHit & hits = rowCollection->hits();
//					checking that there is nothing in padrow > 72
					if (row > 71 && hits.size() != 0) {
						cout << "THERE IS SOMETHING IN UNEXISTING PADROWS!!! (" << row + 1 << ")" << endl;
						return;
					}
					for (int hit = 0; hit < hits.size(); hit++) {
						const StTpcHit * tpcHit = static_cast<const StTpcHit *> (hits[hit]);
						if (tpcHit->flag() == 0) NoHits++;
						NoBadHits++;

						fprintf(hitMap, "%d, %d, %d, %d, %f, %.2f, %.2f\n", sec, row, tpcHit->minPad(), tpcHit->maxPad(), tpcHit->adc(), tpcHit->position().x(), tpcHit->position().y());

						const char * hitColor[6] = {"225, 0, 0", "0, 255, 0", "0, 0, 255", "225, 100, 100" , "225, 239, 95", "125, 125, 125"};
						int hitFlag = tpcHit->flag() > 5 ? 5 :  tpcHit->flag();
					}
				}
			} // Loop over rows in sector 20
		} // loop over sectors

		StSPtrVecTrackNode & trackNode = pEvent->trackNodes();
		int nTracks = trackNode.size();
//		if (nTracks < 100) continue; // removing hits from short tracks that are usually not used in the analysis

		if (!nTracks) continue;

		StTrackNode * node = 0;
		cout << "Number of tracks : " << nTracks << endl;

		for (int track = 0; track < nTracks; track++) {
			node = trackNode[track];
			if (!node) {
				cout << "NO NODE" << endl;
				continue;
			}

//			StGlobalTrack* pTrack = static_cast<StPrimaryTrack*>(node->track(global));
			StPrimaryTrack* pTrack = static_cast<StPrimaryTrack*>(node->track(primary));
			if (!pTrack) continue;
			if (! pTrack->detectorInfo()) {cout << "=============== detectorInfo is missing" << endl; continue;}

			StPtrVecHit phvec = pTrack->detectorInfo()->hits();
			StTrackGeometry * pTrackParams = pTrack->geometry();
			StTrackGeometry * o_pTrackParams = pTrack->outerGeometry();
			if (!pTrackParams) {cout << "-=- No track params for primary track -=-" << endl; continue;}

//			CUTS ------------------------------------------
			if (pTrackParams->momentum().pseudoRapidity() > 0.5) continue;
//			CUTS ------------------------------------------

//			if (pTrack->idTruth() != 8) continue;
//			cout << pTrack->idTruth() << endl;

			int up = -1; // 1 is vertically up y > 0 and -1 is vertically down y < 0

//			BELOW IS THE COLOR SETUP FOR THE HITS ON THE TRACK ACCORDING TO THE STAR COLOR SCHEME
			double int myR, myG, myB;
			double trackP = pTrackParams->momentum().perp();
			double maxP = 4.5;
			double colval = trackP < maxP ? trackP / maxP : 1;
			double colvaltimes4 = colval * 4.0;

			if ( colval < 0.25 ) {
				myG = colvaltimes4;
				myB = myG;
				myB += 1.0 - colvaltimes4;
			}
			else if ( colval < 0.5 ) {
				myG = 1.0 - ( colvaltimes4 - 1.0 );
				myB = myG;
				myG += colvaltimes4 - 1.0;
			}
			else if ( colval < 0.75 ) {
				myR = colvaltimes4 - 2.0;
				myG = myR;
				myG += 1.0 - ( colvaltimes4 - 2.0 );
			}
			else {
				myR = 1.0 - ( colvaltimes4 - 3.0 );
				myG = myR;
				myR += colvaltimes4 - 3.0;
			}
			myR *= 255;
			myG *= 255;
			myB *= 255;

			int myCharge = pTrackParams->charge() == 1 ? 1 : 0;

//			if (phvec.size() < 20) continue; // SELECTION FOR NUMBER OF HITS ON THE TRACK
			bool doIneedThisTrack = true;
			for (int hit = 0; hit < phvec.size(); hit++) {
//				if (hvec[hit]->detector() == kTpcId) {
					StTpcHit *tpcHit = static_cast<StTpcHit *> (phvec[hit]);

					if (!tpcHit) continue;
//					if (tpcHit->flag() != 0) continue;
					if (tpcHit->flag() > 1) continue;

					float hitX = tpcHit->position().x();
					float hitY = tpcHit->position().y();
					float hitZ = tpcHit->position().z();

			}

			double thickness = (o_pTrackParams->origin().z() + 410) * 4/410;
			thickness = thickness < 0 ? 0 : thickness;
			if (pTrackParams->momentum().z() < 0) thickness = TMath::Abs(o_pTrackParams->origin().z()) *0.5 / 200 ;
			double length = TMath::Sqrt(o_pTrackParams->origin().x() * o_pTrackParams->origin().x() + o_pTrackParams->origin().y() * o_pTrackParams->origin().y());
			double segmentLength = pTrackParams->momentum().perp() * 40/2;
			double segmentLength1 = (o_pTrackParams->origin().z() + 410) * segmentLength/410;

			fprintf(dataOut, "%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %d, %d, %d, %d, %.2f, %.2f\n",
					pTrackParams->origin().x(), pTrackParams->origin().y(), pTrackParams->origin().z(),
					o_pTrackParams->origin().x(), o_pTrackParams->origin().y(), o_pTrackParams->origin().z(),
					pTrackParams->origin().x(), pTrackParams->origin().y(), pTrackParams->origin().z(),
					myR, myG,myB, 
					myCharge, 1/pTrackParams->curvature(), pTrackParams->momentum().perp());
		}
		fclose(dataOut);
	} // Event Loop
	fclose(hitMap);
}
