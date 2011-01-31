#ifndef FORMATCONVERTER_H
#define FORMATCONVERTER_H

#include "mmpbsa_exceptions.h"
#include "mmpbsa_io.h"
#include "mmpbsa_utils.h"
#include "structs.h"
#include "GromacsReader.h"
#include "MMPBSAState.h"

//gromacs stuff
#ifdef USE_GROMACS
#include "gromacs/types/idef.h"
#include "gromacs/types/atoms.h"
#endif

namespace mmpbsa_io{

typedef struct {
	size_t curr_snap;
	//for sander trajectories
	std::iostream* sander_crd_stream;
	size_t natoms;
	int ifbox;

	//for gromacs trajectories
	std::string* gromacs_filename;
}trajectory_t;

bool get_next_snap(mmpbsa_io::trajectory_t& traj, std::valarray<mmpbsa_t>& snapshot);

void seek(mmpbsa_io::trajectory_t& traj,const size_t& snap_pos);

void default_trajectory(mmpbsa_io::trajectory_t& traj);
mmpbsa_io::trajectory_t open_trajectory(const std::string& filename);
void destroy_trajectory(mmpbsa_io::trajectory_t& traj);

bool eof(trajectory_t& traj);

std::string get_traj_title(mmpbsa_io::trajectory_t& traj);

void get_gromacs_forcefield(const char* fn,mmpbsa::forcefield_t** split_ff,std::vector<mmpbsa::atom_t>** atom_lists, std::valarray<mmpbsa::MMPBSAState::MOLECULE>& mol_list);

#if 0 // probably want to get rid of these
mmpbsa::SanderParm* gmxtpr2parmtop(const std::string& filename);
mmpbsa::SanderParm* gmxtpr2parmtop(std::iostream& gmxtop);
mmpbsa::SanderParm* gmxtpr2parmtop(const char* fn);
#endif

}//end namespace mmpbsa_io


#endif // TRAJECTORYREADER_H
