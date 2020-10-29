#include <algorithm>
#include <string>
#include <iostream>

template<class T>
    std::string toString(const T& t)
{
     std::ostringstream stream;
     stream << t;
     return stream.str();
}

template<class T>
    T fromString(const std::string& s)
{
     std::istringstream stream (s);
     T t;
     stream >> t;
     return t;
}

double mrnd(double x) { return floor(x * 10.0) / 10.0; };                                                                                                              
double mrnd100(double x) { return floor(x * 100.0) / 100.0; };

void export_event_info_mudst(const int nevents = 1,
//				const char* file = "test.MuDst.root")
//				const char* file = "st_cosmic_adc_19064043_raw_4500010.MuDst.root")
//				const char* file = "st_cosmic_adc_19064043_raw_4500010.event.root")
				const char* file = "/star/data03/daq/2010/003/11003016/st_physics_11003016_raw_1020001.MuDst.root",
				const int eventToSelect = -1
				)

{
  // Load shared libraries
  gROOT->Macro("$STAR/StRoot/StMuDSTMaker/COMMON/macros/loadSharedLibraries.C");
  gROOT->Macro("LoadLogger.C");

  // Load St_db_Maker and co                                                                                                                                           
  gSystem->Load("StDbLib.so");                                                                                                                                         
  gSystem->Load("StDbBroker.so");                                                                                                                                      
  gSystem->Load("St_db_Maker");                                                                                                                                        
                                                                                                                                                                       
  // Load Emc libraries                                                                                                                                                
  gSystem->Load("StDaqLib");                                                                                                                                           
  gSystem->Load("StEmcRawMaker");                                                                                                                                      
  gSystem->Load("StEmcADCtoEMaker"); 

  cout << "Loading shared libraries done" << endl;

  // Create StChain 
  StChain* chain = new StChain; 

  StMuDstMaker* muMk = new StMuDstMaker(0,0,"", file);                       

  // Need St_db_Maker for Emc calibration                                                                                                                              
  St_db_Maker *db1 = new St_db_Maker("db","$HOME/StarDb","MySQL:StarDb","$STAR/StarDb"); 

  // Maker to apply calibration                                                                                                                                        
  StEmcADCtoEMaker *adc_to_e=new StEmcADCtoEMaker();                                                                                                                   
  adc_to_e->setPrint(kFALSE);

  int total = 0;

  chain->Init();
  chain->PrintInfo();

  for (Int_t iev=0;iev < nevents; iev++) {
	chain->Clear();
	int iret = chain->Make(iev);
	total++;
	if (iret) {
		cout << "Bad return code!" << endl;
		break;
	} 
	if (iev < 0) { continue; }

	StMuEvent *event=muMk->muDst()->event(); 

	cout << "Working on eventNumber:\t" << event->eventId() <<endl;

	if (event->eventId() > eventToSelect && eventToSelect != -1) {
		cout << "EVENT YOU ARE LOOKING FOR IS NOT IN THIS FILE!" << endl;
		return 0;
	}

	if (event->eventId() != eventToSelect && eventToSelect != -1) {
		iev --;
		continue;
	}

	Int_t n_glob = muMk->muDst()->GetNGlobalTrack();
	Int_t n_prim = muMk->muDst()->GetNPrimaryTrack();
//	std::cout << iev << ") N GLOBALS: " << n_glob << std::endl;
	std::cout << iev << ") N PRIMARIES: " << n_prim << std::endl;

//	if ( iev != ( nevents - 1 ) ) { continue; } // always grab last event
 
	int MAX_POINTS = 0;	
    StThreeVector<double> xyz;
	double nsig = 0;
    int hypothesis = 0;

	// query offline db for beam information
	// because species type in a file is not reliable
	TSQLServer* db = 0;
	db = TSQLServer::Connect("mysql://db15.star.bnl.gov:3316/RunLog_onl", "test", "");
	if (!db) {
		std::cerr << "ERROR: cannot connect to mysql server" << std::endl;
		return 1;
	}

	TSQLResult* res = 0;
	std::string sql = "SELECT blueSpecies, yellowSpecies FROM `RunLog_onl`.`beamInfo` WHERE beginTime < FROM_UNIXTIME(";
	sql.append(toString(event->eventInfo().time()));
	sql.append(") ORDER BY beginTime DESC LIMIT 1");
	res = db->Query(sql.c_str());
	if (!res) {
		std::cerr << "ERROR: cannot get colliding species information from db" << std::endl;
		return 1;
	}
	TSQLRow* row = 0;
	row = res->Next();
	if (!row) {
		std::cerr << "ERROR: cannot get colliding species information from db" << std::endl;
		return 1;
	}

	TString species[2];
	species[0] = row->GetField(0);
	species[1] = row->GetField(1);
	species[0].ToLower();
	species[1].ToLower();

	if (species[0] == "pp") species[0] = "p";
	if (species[1] == "pp") species[1] = "p";

	db->Close();

	FILE * myStMuDstTrackOut;
	char outFileNameT[200];
	sprintf(outFileNameT, "StTracks_%d_%d.json", event->runId(),  event->eventId());
	outFileT = fopen(outFileNameT, "w");



	ofstream of( (std::string("Track_")+toString(event->runId())+std::string("_")+toString(event->eventId())+std::string(".json")).c_str() );

	of << "{\n"
    << "\t\"EVENT\": { \n"

	   	<< "\t\"runid\": " << event->runNumber() << ",\n"
	   	<< "\t\"evtid\": " << event->eventId() << ",\n"
		<< "\t\"time\": " << event->eventInfo().time() << ",\n"
	   	<< "\t\"type\": \"" << event->eventInfo().type() << "\",\n"
	   	<< "\t\"s_nn\": " << event->runInfo().centerOfMassEnergy() << ",\n"
		<< "\t\"part_yellow\": \"" << species[0] << "\",\n"
		<< "\t\"part_blue\": \"" << species[1] << "\",\n"
		<< "\t\"e_yellow\": " << event->runInfo().beamEnergy(0) << ",\n"
		<< "\t\"e_blue\": " << event->runInfo().beamEnergy(1) << ",\n"
		<< "\t\"pv\": [" 	<< event->primaryVertexPosition().x() << ","
					<< event->primaryVertexPosition().y() << ","
					<< event->primaryVertexPosition().z() << "]\n"
		<< "},\n\n";


    of << "\"TRACKS\": {\n";
    of << "\"tracks\": [\n";

	fprintf(outFileT, "{\"EVENT\": {	\"R\": %d, 	\"Evt\": %d, 	\"B\": 0.5, 	\"tm\": 1528087733 },\n", event->runId(), event->eventId());
	fprintf(outFileT, "\t\"META\": {\n\t\t\"TRACKS\": {\"type\": \"3D\", \n\"tracks\": { \n\t\t\t\"size\":5, \"r_min\": 500, \"r_max\": 2000\n\t\t\t}\n\t\t}\n\t},");
	fprintf(outFileT, "\"TRACKS\": { \"tracks\":[\n");

//	for ( int i = 0; i < n_glob; i++ ) {
	for ( int i = 0; i < n_prim; i++ ) {
//		StMuTrack *track = muMk->muDst()->globalTracks(i);
		StMuTrack *track = muMk->muDst()->primaryTracks(i);
		StPhysicalHelixD helix = track->helix();
		float length = track->lengthMeasured();

		int myCharge = track->charge() == 1 ? 1 : 0;

//			fprintf(svgOutT, "<path d=\"M 0 0 A %.2f %.2f, 0, 0, %d, %.2f %.2f \" fill=\"none\" stroke-width=\"1\" style=\"stroke:rgb(%d, %d, %d)\"></path>\n",  1/pTrackParams->curvature(), 1/pTrackParams->curvature(), myCharge, o_pTrackParams->origin().x(), o_pTrackParams->origin().y(), myR, myG, myB);


		if (length > 0 && track->pt() > 0.01) {
			StThreeVectorF p = track->p();

//			std::cout << "**************** GOOD TRACK *************************\n";

			printf("<path d=\"M %.2f %.2f A %.2f %.2f, 0, 0, %d, %.2f %.2f \" fill=\"none\" stroke-width=\"1\" style=\"stroke:rgb(0, 0, 0)\"></path>\n",  helix.origin().x(), helix.origin().y(), 1/helix.curvature(), 1/helix.curvature(), myCharge, helix.at(track->length()).x(), helix.at(track->length()).y());

			fprintf(outFileT, "{\"pt\": %.3f,\"xyz\":[%.3f, %.3f, %.3f], \"pxyz\":[%.3f, %.3f, %.3f], \"q\": %d,\"l\": %.3f,\"nh\":20},\n", track->pt(),  helix.origin().x(), helix.origin().y(), helix.origin().z(), track->momentum().x(), track->momentum().y(), track->momentum().z(), track->charge(), track->length());
/*
			std::cout << helix.at(30) << std::endl;
			std::cout << "p: " << p << ", pt: " << track->pt() << ", HELIX: \n" << "curvature: " << helix.curvature() << ", dip: " << helix.dipAngle() 
				<< ", phase: " << helix.phase() << ", h: " << helix.h() 
				<< ", o.x: " << helix.origin().x()  
				<< ", o.y: " << helix.origin().y()  
				<< ", o.z: " << helix.origin().z()
				<< ", at(30) " << helix.at(30) << std::endl;  
*/
		}

		MAX_POINTS = 4*length*helix.curvature() + 1;		
		nsig = 1000;
		hypothesis = 0;
		if (fabs(track->nSigmaElectron()) < nsig) {
			nsig = fabs(track->nSigmaElectron());
			hypothesis = 1;
		}
		if (fabs(track->nSigmaPion()) < nsig) {
			nsig = track->nSigmaPion();
			hypothesis = 2;
		}
		if (fabs(track->nSigmaProton()) < nsig) {
			nsig = track->nSigmaProton();
			hypothesis = 3;
		}
		if (fabs(track->nSigmaKaon()) < nsig) {
			nsig = track->nSigmaKaon();
			hypothesis = 4;
		}
		
		float pt = length / float(MAX_POINTS);

		if ( i != 0 ) {
		of << ",{ "; 
		} else {
		of << "{ "; 
		}
		of << "\"pt\": " << TMath::Floor(track->pt() * 1000 +  0.5) / 1000 << ",";
		of << "\"t\": " << hypothesis << ", ";
		of << "\"e\": " << TMath::Floor(track->eta() * 1000 +  0.5) / 1000 << ",";
		of << "\"p\": " << TMath::Floor(track->phi() * 1000 +  0.5) / 1000 << ",";
		if (track->primaryTrack()) {
			of << "\"pr\": " << 1 << ",";
		} else {
			of << "\"pr\": " << 0 << ",";
		}
		of << "\"c\": " << track->charge() << ",";
		of << "\"pts\": [";

		for (int j = 0; j <= MAX_POINTS; j++) {
			xyz = helix.at(j*pt);
			if (xyz.perp() < 45.0 || xyz.perp() > 205.0) break; // point is outside of TPC range
			if ( j != 0 ) {
				of << ", [" << int(xyz.x()) << "," << int(xyz.y()) << "," << int(xyz.z()) << "]";
			} else {
				of << "[" << int(xyz.x()) << "," << int(xyz.y()) << "," << int(xyz.z()) << "]";
			}
		}
		of << "]}\n";
	}
	of << "]\n }\n }\n";
	of.close();
	//exit(0);
	fprintf(outFileT, "\n]\n}\n}");
  } 
//  chain->Finish(); 
  cout << "****************************************** " << endl;
  cout << "total number of events  " << total << endl;
  cout << "****************************************** " << endl; 

}
