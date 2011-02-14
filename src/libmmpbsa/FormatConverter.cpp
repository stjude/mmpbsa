#include "FormatConverter.h"


size_t gromacs_num_bond_types(const std::map<t_functype,size_t>& function_sizes)
{
	std::map<t_functype,size_t>::const_iterator function;
	size_t returnMe = 0;
	for(function = function_sizes.begin();function != function_sizes.end();function++)
	{
		switch(function->first)
		{
		case F_BONDS:
			returnMe += function->second;
		default:
			continue;
		}
	}
	return returnMe;
}

size_t gromacs_num_angle_types(const std::map<t_functype,size_t>& function_sizes)
{
	std::map<t_functype,size_t>::const_iterator function;
	size_t returnMe = 0;
	for(function = function_sizes.begin();function != function_sizes.end();function++)
	{
		switch(function->first)
		{
		case F_ANGLES:
			returnMe += function->second;
		default:
			continue;
		}
	}
	return returnMe;
}

size_t gromacs_num_dihedral_types(const std::map<t_functype,size_t>& function_sizes)
{
	std::map<t_functype,size_t>::const_iterator function;
	size_t returnMe = 0;
	for(function = function_sizes.begin();function != function_sizes.end();function++)
	{
		switch(function->first)
		{
		case F_PDIHS:case F_PIDIHS:
			returnMe += function->second;
		default:
			continue;
		}
	}
	return returnMe;
}

size_t gromacs_num_lj_parameters(const std::map<t_functype,size_t>& function_sizes)
{
	std::map<t_functype,size_t>::const_iterator function;
	size_t returnMe = 0;
	for(function = function_sizes.begin();function != function_sizes.end();function++)
	{
		switch(function->first)
		{
		case F_LJ:
			returnMe += function->second;
		default:
			continue;
		}
	}
	return returnMe;
}

void gromacs_adjust_start_indices(std::map<t_functype,size_t>& start_indices,const std::map<t_functype,size_t>& function_sizes)
{
	std::map<t_functype,size_t>::iterator index = start_indices.begin();
	for(;index != start_indices.end();index++)
	{
		switch(index->first)
		{
		case F_PIDIHS:
		{
			if(function_sizes.find(F_PDIHS) != function_sizes.end())
				index->second -= function_sizes.find(F_PDIHS)->second;//F_PDIHS before F_PIDIHS
			break;
		}
		}
	}
}

/**
 * Sets up the forcefield data.
 *
 * mol_list is created as a mask for atom_lists, as to whether an atom is in the Receptor or ligand
 *
 * It is assumed that the first molblock is the receptor and the rest are ligand, except for those named "SOL"
 * This should be changed.
 */
void mmpbsa_io::get_gromacs_forcefield(const char* fn,mmpbsa::forcefield_t** split_ff,std::vector<mmpbsa::atom_t>** atom_lists, std::valarray<mmpbsa::MMPBSAState::MOLECULE>& mol_list)
{
	using namespace mmpbsa;
	mmpbsa_io::gromacs_idx_offsets offsets;
	size_t atom_offset = 0;
	//unused
//	FILE *gp;
	int         fp,atot;
//	t_state     state;
	rvec        *f=NULL;
//	t_inputrec  ir;
	t_tpxheader tpx;
//	t_topology top;
	gmx_mtop_t  mtop;
//	gmx_groups_t *groups;
//	gmx_ffparams_t* force_field;
	const char* mdpfn = 0;
	bool bSysTop,bShowNumbers;
	bSysTop = false;
	bShowNumbers = true;

	static const mmpbsa_t joules2cal = 0.23889;
	static const mmpbsa_t angst2nm_sqrd = 0.01;
	static const mmpbsa_t nm2angst_6 = 1e+6;
	static const mmpbsa_t nm2angst_12 = 1e+12;
 	static const mmpbsa_t nm2angst = 10;
 	static const mmpbsa_t charge_units = 18.2182634799;

	if(split_ff == 0 || atom_lists == 0)
		throw mmpbsa::MMPBSAException("mmpbsa::MMPBSAException: Null pointer supplied for atom list and/or force field.",mmpbsa::NULL_POINTER);

	*split_ff = new mmpbsa::forcefield_t[MMPBSAState::END_OF_MOLECULES];
	*atom_lists = new std::vector<mmpbsa::atom_t>[MMPBSAState::END_OF_MOLECULES];

	//for simplicity alias these here.
	forcefield_t& complex = split_ff[0][MMPBSAState::COMPLEX];
	forcefield_t& receptor = split_ff[0][MMPBSAState::RECEPTOR];
	forcefield_t& ligand = split_ff[0][MMPBSAState::LIGAND];
	for(size_t i = 0;i<MMPBSAState::END_OF_MOLECULES;i++)
	{
		init(&split_ff[0][i]);
	}


	if(fn == 0 || strlen(fn) == 0)
		throw mmpbsa::MMPBSAException("mmpbsa_io::gmxtpr2parmtop: Filename is required.",mmpbsa::DATA_FORMAT_ERROR);

	//const std::vector<size_t>& interaction_types = mmpbsa_io::allowed_gmx_energies();
	std::vector<size_t>::const_iterator interaction_type;

	std::map<t_functype,size_t> function_start_index,function_sizes;

	read_tpx(fn,NULL,NULL,&atot,0,0,0,&mtop);
	if (!mdpfn) {
		//top = gmx_mtop_t_to_t_topology(&mtop);
		if (available(stdout,&tpx,0,fn)) {
			//setup constants
			init(offsets);

			//Determine beginning of function type data. This will be used to
			//create dynamic arrays of interaction data.
			for(size_t i = 0;i<mtop.ffparams.ntypes;i++)//get offset values
			{
				if(function_sizes.find(mtop.ffparams.functype[i]) == function_sizes.end())
				{
					function_sizes[mtop.ffparams.functype[i]] = 1;
					function_start_index[mtop.ffparams.functype[i]] = i;
				}
				else
					function_sizes[mtop.ffparams.functype[i]]++;

			}

			//Allocate memory of energy data.
			complex.bond_energy_data = new bond_energy_t[gromacs_num_bond_types(function_sizes)];
			complex.angle_energy_data = new bond_energy_t[gromacs_num_angle_types(function_sizes)];
			complex.dihedral_energy_data = new dihedral_energy_t[gromacs_num_dihedral_types(function_sizes)];
			complex.lj_params.reserve(gromacs_num_lj_parameters(function_sizes));

			//Adjust start indices so that bond types may be placed in above allocated arrays
			//with similar bond types without overlapping.
			gromacs_adjust_start_indices(function_start_index,function_sizes);

			//Load energy data
			for(size_t i = 0;i<mtop.ffparams.ntypes;i++)//get offset values
			{
				switch(mtop.ffparams.functype[i])
				{
				case F_BONDS:
				{
					bond_energy_t new_energy;
					new_energy.energy_const = 0.5*mtop.ffparams.iparams[i].harmonic.krA*joules2cal*angst2nm_sqrd;
					new_energy.eq_distance = mtop.ffparams.iparams[i].harmonic.rA*nm2angst;
					complex.bond_energy_data[i-function_start_index[mtop.ffparams.functype[i]]] = new_energy;
					break;
				}
				case F_ANGLES:
				{	bond_energy_t new_energy;
					new_energy.energy_const = 0.5*mtop.ffparams.iparams[i].harmonic.krA*joules2cal;
					new_energy.eq_distance = MMPBSA_DEG_TO_RAD*mtop.ffparams.iparams[i].harmonic.rA;
					complex.angle_energy_data[i-function_start_index[mtop.ffparams.functype[i]]] = new_energy;
					break;
				}
				case F_PDIHS: case F_PIDIHS:
				{
					dihedral_energy_t new_energy;
					new_energy.energy_const = joules2cal*mtop.ffparams.iparams[i].pdihs.cpA;
					new_energy.periodicity = mtop.ffparams.iparams[i].pdihs.mult;
					new_energy.phase = MMPBSA_DEG_TO_RAD*mtop.ffparams.iparams[i].pdihs.phiA;
					complex.dihedral_energy_data[i-function_start_index[mtop.ffparams.functype[i]]] = new_energy;
					break;
				}
				case F_LJ:
					lj_params_t lj_params;
					lj_params.c12 = nm2angst_12*joules2cal*mtop.ffparams.iparams[i].lj.c12;
					lj_params.c6 = nm2angst_6*joules2cal*mtop.ffparams.iparams[i].lj.c6;
					complex.lj_params.push_back(lj_params);
					break;
				}//end switch
			}//end loading interaction data.

			//load molecule data
			for(size_t mol_block = 0;mol_block <mtop.nmolblock;mol_block++)
			{
				if(mtop.moltype[mtop.molblock[mol_block].type].name != 0 && (strcmp(*mtop.moltype[mtop.molblock[mol_block].type].name,"SOL") == 0/* || strcmp(*mtop.moltype[mtop.molblock[mol_block].type].name,"CL") == 0*/))// NO SOLVENTS!!!
					continue;
				for(size_t ith_mol_block = 0;ith_mol_block<mtop.molblock[mol_block].nmol;ith_mol_block++)
				{
					gmx_moltype_t& mol = mtop.moltype[mtop.molblock[mol_block].type];
					forcefield_t * curr_field;
					std::vector<mmpbsa::atom_t>* curr_atom_list;
					if(mol_block == 0)
					{
						curr_field = &split_ff[0][MMPBSAState::RECEPTOR];
						curr_atom_list = &atom_lists[0][MMPBSAState::RECEPTOR];
					}
					else
					{
						curr_field = &split_ff[0][MMPBSAState::LIGAND];
						curr_atom_list = &atom_lists[0][MMPBSAState::LIGAND];
					}

					//load atom data
					for(size_t atom_idx = 0;atom_idx<mol.atoms.nr;atom_idx++)
					{
						t_atom& atom = mol.atoms.atom[atom_idx];
						mmpbsa::atom_t new_atom;
						new_atom.atom_type = atom.typeB;
						new_atom.atomic_number = atom.atomnumber;
						new_atom.charge = charge_units*atom.qB;
						new_atom.name = (*mol.atoms.atomname[atom_idx]);
						for(size_t ex_idx = mol.excls.index[atom_idx];ex_idx < mol.excls.index[atom_idx+1];ex_idx++)
							new_atom.exclusion_list.insert(mol.excls.a[ex_idx]);
						curr_atom_list->push_back(new_atom);

						//exclusion list may need to be revised if there is an atom index offset for the complex
						if(atom_offset != 0)
						{
							new_atom.exclusion_list.clear();
							for(size_t ex_idx = mol.excls.index[atom_idx];ex_idx < mol.excls.index[atom_idx+1];ex_idx++)
								new_atom.exclusion_list.insert(mol.excls.a[ex_idx] + atom_offset);
						}
						atom_lists[0][MMPBSAState::COMPLEX].push_back(new_atom);
					}
					//load interaction indices
					for(size_t erg_type = 0;erg_type < F_NRE;erg_type++)
					{
						for(size_t erg_idx = 0;erg_idx < mol.ilist[erg_type].nr + mol.ilist[erg_type].nr_nonperturbed;erg_idx += interaction_function[erg_type].nratoms + 1)
						{
							switch(erg_type)
							{
							case F_BONDS:
							{
								bond_t new_bond;
								new_bond.atom_i = mol.ilist[erg_type].iatoms[erg_idx + 1];
								new_bond.atom_j = mol.ilist[erg_type].iatoms[erg_idx + 2];
								new_bond.bond_energy = &complex.bond_energy_data[mol.ilist[erg_type].iatoms[erg_idx] - function_start_index[erg_type]];
								curr_field->bonds_with_H.push_back(new_bond);

								//add index offset if needs for complex
								new_bond.atom_i += atom_offset;
								new_bond.atom_j += atom_offset;
								complex.bonds_with_H.push_back(new_bond);
								break;
							}
							case F_ANGLES:
							{
								angle_t new_bond;
								new_bond.atom_i = mol.ilist[erg_type].iatoms[erg_idx + 1];
								new_bond.atom_j = mol.ilist[erg_type].iatoms[erg_idx + 2];
								new_bond.atom_k = mol.ilist[erg_type].iatoms[erg_idx + 3];
								new_bond.angle_energy = &complex.angle_energy_data[mol.ilist[erg_type].iatoms[erg_idx] - function_start_index[erg_type]];
								curr_field->angles_with_H.push_back(new_bond);

								//add index offset if needs for complex
								new_bond.atom_i += atom_offset;
								new_bond.atom_j += atom_offset;
								new_bond.atom_k += atom_offset;
								complex.angles_with_H.push_back(new_bond);
								break;
							}
							case F_PDIHS: case F_PIDIHS:
							{
								dihedral_t new_dihedral;
								new_dihedral.atom_i = mol.ilist[erg_type].iatoms[erg_idx + 1];
								new_dihedral.atom_j = mol.ilist[erg_type].iatoms[erg_idx + 2];
								new_dihedral.atom_k = mol.ilist[erg_type].iatoms[erg_idx + 3];
								new_dihedral.atom_l = mol.ilist[erg_type].iatoms[erg_idx + 4];
								new_dihedral.lj.c12 = new_dihedral.lj.c6 = 0;
								new_dihedral.dihedral_energy = &complex.dihedral_energy_data[mol.ilist[erg_type].iatoms[erg_idx] - function_start_index[erg_type]];
								curr_field->dihedrals_with_H.push_back(new_dihedral);

								new_dihedral.atom_i += atom_offset;
								new_dihedral.atom_j += atom_offset;
								new_dihedral.atom_k += atom_offset;
								new_dihedral.atom_l += atom_offset;
								complex.dihedrals_with_H.push_back(new_dihedral);
								break;
							}
							case F_LJ14:
								dihedral_t new_dihedral;
								new_dihedral.atom_i = mol.ilist[erg_type].iatoms[erg_idx + 1];
								new_dihedral.atom_l = mol.ilist[erg_type].iatoms[erg_idx + 2];
								new_dihedral.atom_j = new_dihedral.atom_k = -1;
								new_dihedral.lj.c12 = nm2angst_12*joules2cal*mtop.ffparams.iparams[mol.ilist[erg_type].iatoms[erg_idx]].lj14.c12A;
								new_dihedral.lj.c6 = nm2angst_6*joules2cal*mtop.ffparams.iparams[mol.ilist[erg_type].iatoms[erg_idx]].lj14.c6A;
								new_dihedral.dihedral_energy = 0;
								curr_field->dihedrals_with_H.push_back(new_dihedral);

								new_dihedral.atom_i += atom_offset;
								new_dihedral.atom_l += atom_offset;
								complex.dihedrals_with_H.push_back(new_dihedral);
								break;
							case F_LJ:
								break;
							default:
								std::cerr << "get_gromacs_forcefield: Warning: Non supported gromacs interaction type: " << erg_type << std::endl;
								break;
							}//end switch
						}//end energy loop
					}//end outer energy loop
					atom_offset += mol.atoms.nr;
				}
			}
		}
	}

	//done_state(&state);
	sfree(f);

	//create mol_list
	size_t complex_size = atom_lists[0][MMPBSAState::COMPLEX].size();
	mol_list.resize(complex_size,MMPBSAState::LIGAND);
	mol_list[std::slice(0,atom_lists[0][MMPBSAState::RECEPTOR].size(),1)] = MMPBSAState::RECEPTOR;

	//All molecules must share the LJ paramters
	receptor.lj_params = ligand.lj_params = complex.lj_params;
}



