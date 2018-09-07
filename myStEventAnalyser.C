void myStEventAnalyser(const char * file) {
	TFile * outRoot = new TFile("testOut.root", "RECREATE");

	FILE * outFile;
	FILE * outFile_iTPC;
	FILE * outFile_TPC;
	char outFileName[100];
	char outFileName_iTPC[100];
	char outFileName_TPC[100];

//	const char * file = "st_cosmic_adc_19053068_raw_2000015.event.root";
	gROOT->LoadMacro("$STAR/StRoot/StMuDSTMaker/COMMON/macros/loadSharedLibraries.C");
	loadSharedLibraries();

	gROOT->LoadMacro("bfc.C");
	TString Chain("StEvent, nodefault, mysql, in");
//	TString Chain("StEvent, nodefault, mysql, in, quiet");
	bfc(0, Chain, file); // This will make chain

	for (int event = 0; event < 10000000; event++) {
		int iMake = chain->MakeEvent();
		if (iMake % 10 == kStEOF || iMake % 10==kStFatal) break;

		StEvent * pEvent = (StEvent*) chain->GetInputDS("StEvent");
		if (event % 1000 == 0) cout << pEvent->id() << endl;
//		if (pEvent->id() > 53000 && pEvent->id() < 54000) cout << "Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << endl;
//		if (pEvent->id() != 194093) continue;
//
//		if (pEvent->id() != 2420421) {
//			cout << pEvent->id() << endl;
//			continue; // JUST AN EVENT SELECTOR, REMOVE WHEN NOT NEEDED
//		}

		else cout << "Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << endl;
		sprintf(outFileName, "StEvent_%d_%d.csv", pEvent->runId(),  pEvent->id());
		sprintf(outFileName_iTPC, "StEvent_%d_%d_iTPC.json", pEvent->runId(),  pEvent->id());
		sprintf(outFileName_TPC, "StEvent_%d_%d_TPC.json", pEvent->runId(),  pEvent->id());

		StTpcHitCollection* TpcHitCollection = pEvent->tpcHitCollection();
		if (!TpcHitCollection) {
			cout << "No TPC Hit Collection" << endl;
			return;
		}
		unsigned int numberOfSectors = TpcHitCollection->numberOfSectors();
		StTpcSectorHitCollection* sectorCollection = TpcHitCollection->sector(19); //Get sector 20

		unsigned int numberOfPadrows = sectorCollection->numberOfPadrows();
		unsigned long NoHits = 0;
		unsigned long NoBadHits = 0;
		bool doOnce = false;
		for (int row = 0; row < numberOfPadrows; row++) {
			StTpcPadrowHitCollection *rowCollection = sectorCollection->padrow(row);
			if (rowCollection) {
				StSPtrVecTpcHit & hits = rowCollection->hits();
//				checking that there is nothing in padrow > 72
				if (row > 71 && hits.size() != 0) {
					cout << "THERE IS SOMETHING IN UNEXISTING PADROWS!!! (" << row + 1 << ")" << endl;
					return;
				}
				for (int hit = 0; hit < hits.size(); hit++) {
					const StTpcHit * tpcHit = static_cast<const StTpcHit *> (hits[hit]);
					if (tpcHit->flag() == 0) NoHits++;
					NoBadHits++;
				}
			}
		} // Loop over rows in sector 20

//		cout << "\t NoHits = " << NoHits << endl;
//		cout << "\t NoBadHits = " << NoBadHits << endl;
		if (NoHits < 20) {
			cout << "Less then 20 hits in sector 20 : "  << NoHits << endl;
			continue;
		}

		StSPtrVecTrackNode & trackNode = pEvent->trackNodes();
		int nTracks = trackNode.size();
		if (nTracks < 20) continue; // removing hits from short tracks that are usually not used in the analysis
		StTrackNode * node = 0;
		for (int track = 0; track < nTracks; track++) {
			node = trackNode[track];
			if (!node) continue;
			StPrimaryTrack* pTrack = static_cast<StPrimaryTrack*>(node->track(primary));
			if (!pTrack) continue;
//			cout << *gTrack << endl;
			if (! pTrack->detectorInfo()) {cout << "=============== detectorInfo is missing" << endl; continue;}
			StPtrVecHit phvec = pTrack->detectorInfo()->hits();

//			if (hvec.size() < 70) continue;

//			if (track != 113 && track != 514 && track != 559) continue; 

			cout << "Event: Run "<< pEvent->runId() << " Event No: " << pEvent->id() << endl;
			cout << "Track number : " << track << endl;

			for (int hit = 0; hit < phvec.size(); hit++) {
//				if (hvec[hit]->detector() == kTpcId) {
					StTpcHit *tpcHit = static_cast<StTpcHit *> (phvec[hit]);

//					if (!tpcHit || tpcHit->sector() != 20) continue;
					if (!tpcHit) continue;
					if (tpcHit->flag() != 0) continue;
					if(!doOnce) {
						outFile = fopen(outFileName, "w");
						outFile_iTPC = fopen(outFileName_iTPC, "w");
						outFile_TPC = fopen(outFileName_TPC, "w");
						doOnce = true;
					}
					float hitX = tpcHit->position().x();
					float hitY = tpcHit->position().y();
					float hitZ = tpcHit->position().z();
//					cout << tpcHit->pad() << ", " << tpcHit->padrow() << ", " << tpcHit->timeBucket() << ", " <<tpcHit->adc() << ", " << tpcHit->position().x() << endl;
//					if (hitZ > 0) continue;
//					if (tpcHit->padrow() > 40) fprintf(outFile_TPC, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
//					else fprintf(outFile_iTPC, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
					fprintf(outFile, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
//					cout << "[" << hitX << ", " << hitY << ", " << hitZ << "]," << endl;
//					fprintf(outFile, "%d, %d, %d, %d, %f, %f, %f, %f\n", track, tpcHit->pad(), tpcHit->padrow(), tpcHit->timeBucket(), tpcHit->adc(), hitX, hitY, hitZ);
//				}
			}
/*			
			StGlobalTrack* gTrack = static_cast<StGlobalTrack*>(node->track(global));
			if (!pTrack) continue;
			if (! gTrack->detectorInfo()) {cout << "=============== detectorInfo is missing" << endl; continue;}
			StPtrVecHit ghvec = gTrack->detectorInfo()->hits();
			for (int hit = 0; hit < ghvec.size(); hit++) {
//				if (hvec[hit]->detector() == kTpcId) {
					StTpcHit *tpcHit = static_cast<StTpcHit *> (ghvec[hit]);

//					if (!tpcHit || tpcHit->sector() != 20) continue;
					if (!tpcHit) continue;
					if (tpcHit->flag() != 0) continue;
					if(!doOnce) {
						outFile = fopen(outFileName, "w");
						outFile_iTPC = fopen(outFileName_iTPC, "w");
						outFile_TPC = fopen(outFileName_TPC, "w");
						doOnce = true;
					}
					float hitX = tpcHit->position().x();
					float hitY = tpcHit->position().y();
					float hitZ = tpcHit->position().z();
//					cout << tpcHit->pad() << ", " << tpcHit->padrow() << ", " << tpcHit->timeBucket() << ", " <<tpcHit->adc() << ", " << tpcHit->position().x() << endl;
//					if (hitZ > 0) continue;
//					if (tpcHit->padrow() > 40) fprintf(outFile_TPC, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
//					else fprintf(outFile_iTPC, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
					fprintf(outFile, "[%.2f, %.2f, %.2f],\n", hitX, hitY, hitZ);
//					cout << "[" << hitX << ", " << hitY << ", " << hitZ << "]," << endl;
//					fprintf(outFile, "%d, %d, %d, %d, %f, %f, %f, %f\n", track, tpcHit->pad(), tpcHit->padrow(), tpcHit->timeBucket(), tpcHit->adc(), hitX, hitY, hitZ);
//				}
			}
*/		

		}
		if (doOnce) {
			fclose(outFile);
			fclose(outFile_iTPC);
			fclose(outFile_TPC);
		}
	} // Event Loop
}
