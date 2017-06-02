/* ---------------------------------------------------------------------
 *
 * Copyright (C) 2005 - 2015 by the deal.II authors
 *
 * This file is part of the deal.II library.
 *
 * The deal.II library is free software; you can use it, redistribute
 * it, and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * The full text of the license can be found in the file LICENSE at
 * the top level of the deal.II distribution.
 *
 * ---------------------------------------------------------------------

 *
 * Author: Wolfgang Bangerth, Texas A&M University, 2005, 2006
 */



#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/logstream.h>
#include <deal.II/base/function.h>
#include <deal.II/lac/block_vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/block_sparse_matrix.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/lac/iterative_inverse.h>

#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_renumbering.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/fe/fe_dgq.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/data_out.h>

#include <deal.II/lac/sparse_direct.h>

#include <deal.II/lac/sparse_ilu.h>

#include <fstream>
#include <iostream>

#include <deal.II/fe/fe_raviart_thomas.h>
#include <deal.II/numerics/data_postprocessor.h>
#include <deal.II/base/tensor_function.h>

namespace Step20
{
  using namespace dealii;
    using namespace numbers;
  namespace data
  {
  	  const int problem_degree = 2;
  	  const int refinement_level = 3;
  	  const int dimension = 2;
  			  
      const double rho_f = 1.0;
      const double eta = 1.0;
      
      const double top = 1.0;
      const double bottom = 0.0;      
      const double left = 0.0;
      const double right = PI;
     
      const double lambda = 1.0;


  }

  template <int dim>
  class MixedLaplaceProblem
  {
  public:
    MixedLaplaceProblem (const unsigned int degree);
    void run ();

  private:
    void make_grid_and_dofs ();
    void assemble_system ();
    void solve ();
    void compute_errors () const;
    void output_results () const;
      void calculate_vf();

    const unsigned int   degree;

    Triangulation<dim>   triangulation;
    FESystem<dim>        fe;
    DoFHandler<dim>      dof_handler;
    ConstraintMatrix     constraints;

    BlockSparsityPattern      sparsity_pattern;
    BlockSparseMatrix<double> system_matrix;

    BlockVector<double>       solution;
    BlockVector<double>       system_rhs;
    Vector<double>        vf;
    Tensor<1,dim>         grad_pf;
      
      FESystem<dim> fe2;
      DoFHandler<dim> dof_handler2;
      ConstraintMatrix hanging_node_constraints;
      SparsityPattern sparsity_pattern2;
      SparseMatrix<double> system_matrix_vf;
      Vector<double> solution_vf;
      Vector<double> system_rhs_vf;
  
  };

    
  template <int dim>
  class RightHandSide : public Function<dim>
  {
  public:
    RightHandSide () : Function<dim>(1) {}

    virtual double value (const Point<dim>   &p,
                          const unsigned int  component = 0) const;
  };


  template <int dim>
  class PressureBoundaryValues : public Function<dim>
  {
  public:
    PressureBoundaryValues () : Function<dim>(1) {}

    virtual double value (const Point<dim>   &p,
                          const unsigned int  component = 0) const;
  };


  template <int dim>
  class ExactSolution : public Function<dim>
  {
  public:
    ExactSolution () : Function<dim>(dim+1) {}

    virtual void vector_value (const Point<dim> &p,
                               Vector<double>   &value) const;
  };


  template <int dim>
  double RightHandSide<dim>::value (const Point<dim>  &p,
                                    const unsigned int /*component*/) const
  {

      const double permeability = (0.5*std::sin(3*p[0])+1)
      * std::exp( -100.0*(p[1]-0.5)*(p[1]-0.5) );
      
      
      return 2*p[1]* data::rho_f;
  }



  template <int dim>
  double PressureBoundaryValues<dim>::value (const Point<dim>  &p,
                                             const unsigned int /*component*/) const
  {

	  
   return -2./3*data::rho_f;
  }



  template <int dim>
  void
  ExactSolution<dim>::vector_value (const Point<dim> &p,
                                    Vector<double>   &values) const
  {
    Assert (values.size() == dim+1,
            ExcDimensionMismatch (values.size(), dim+1));

      
      const double permeability = 1.0;
      
    values(0) = 0.0;
      values(1) = data::rho_f*(1-p[1]*p[1])*permeability;
    values(2) = -data::rho_f*(p[1] - (1.0/3.0)*p[1]*p[1]*p[1]);
  }




  template <int dim>
  class KInverse : public TensorFunction<2,dim>
  {
  public:
    KInverse () : TensorFunction<2,dim>() {}

    virtual void value_list (const std::vector<Point<dim> > &points,
                             std::vector<Tensor<2,dim> >    &values) const;
  };


    template <int dim>
    void
    KInverse<dim>::value_list (const std::vector<Point<dim> > &points,
                               std::vector<Tensor<2,dim> >    &values) const
    {
        
         Assert (points.size() == values.size(),
          
         
          ExcDimensionMismatch (points.size(), values.size()));
        for (unsigned int p=0; p<points.size(); ++p)
        {
            values[p].clear ();

            const double permeability = 1.0;
                
            for (unsigned int d=0; d<dim; ++d)
                values[p][d][d] = permeability;
        }
    }
    
    

  template <int dim>
  class K : public TensorFunction<2,dim>
  {
  public:
    K () : TensorFunction<2,dim>() {}

    virtual void value_list (const std::vector<Point<dim> > &points,
                             std::vector<Tensor<2,dim> >    &values) const;
  };


  template <int dim>
  void
  K<dim>::value_list (const std::vector<Point<dim> > &points,
                             std::vector<Tensor<2,dim> >    &values) const
  {
    Assert (points.size() == values.size(),
            ExcDimensionMismatch (points.size(), values.size()));

    for (unsigned int p=0; p<points.size(); ++p)
      {
          values[p].clear ();
          
          
          Assert (points.size() == values.size(),
                  ExcDimensionMismatch (points.size(), values.size()));
          for (unsigned int p=0; p<points.size(); ++p)
          {
              values[p].clear ();/*
                                  const double distance_to_flowline
                                  = std::fabs(points[p][1]-0.2*std::sin(10*points[p][0]));*/
              
             
              const double permeability = 1.0;
              
              for (unsigned int d=0; d<dim; ++d)
                  values[p][d][d] = 1./permeability;
          }

      }
  }

    
  template <int dim>
  MixedLaplaceProblem<dim>::MixedLaplaceProblem (const unsigned int degree)
    :
    degree (degree),
    fe (FE_RaviartThomas<dim>(degree), 1,
        FE_DGQ<dim>(degree), 1),
    dof_handler (triangulation),
    fe2 (FE_DGQ<dim>(degree), dim),
    dof_handler2 (triangulation)
  {}




  template <int dim>
  void MixedLaplaceProblem<dim>::make_grid_and_dofs ()
  {
      {
          std::vector<unsigned int> subdivisions (dim, 1);
          subdivisions[0] = 4;

          const Point<dim> bottom_left = (dim == 2 ?
                                          Point<dim>(data::left,data::bottom) :
                                          Point<dim>(-2,0,-1));
          const Point<dim> top_right   = (dim == 2 ?
                                          Point<dim>(data::right,data::top) :
                                          Point<dim>(0,1,0));

          GridGenerator::subdivided_hyper_rectangle (triangulation,
                                                     subdivisions,
                                                     bottom_left,
                                                     top_right);
      }


      for (typename Triangulation<dim>::active_cell_iterator
           cell = triangulation.begin_active();
           cell != triangulation.end(); ++cell)
          for (unsigned int f=0; f<GeometryInfo<dim>::faces_per_cell; ++f)
              if (cell->face(f)->center()[dim-1] == data::top)
                  cell->face(f)->set_all_boundary_ids(1);
              else if (cell->face(f)->center()[dim-1] == data::bottom)
                  cell->face(f)->set_all_boundary_ids(2);


    triangulation.refine_global (data::refinement_level);

    dof_handler.distribute_dofs (fe);
 
    
 DoFRenumbering::component_wise (dof_handler);
    std::vector<types::global_dof_index> dofs_per_component (dim+1);
    DoFTools::count_dofs_per_component (dof_handler, dofs_per_component);
    const unsigned int n_u = dofs_per_component[0],
                       n_p = dofs_per_component[dim];
      

    std::cout << "Problem Degree: "
    		  << data::problem_degree
              << std::endl
			  << "Refinement level: "
			  << data::refinement_level
			  << std::endl
			  << "Number of active cells: "
              << triangulation.n_active_cells()
              << std::endl
              << "Total number of cells: "
              << triangulation.n_cells()
              << std::endl
              << "Number of degrees of freedom: "
              << dof_handler.n_dofs()
              << " (" << n_u << '+' << n_p << ')'
              << std::endl;

    BlockDynamicSparsityPattern dsp(2, 2);
    dsp.block(0, 0).reinit (n_u, n_u);
    dsp.block(1, 0).reinit (n_p, n_u);
    dsp.block(0, 1).reinit (n_u, n_p);
    dsp.block(1, 1).reinit (n_p, n_p);
    dsp.collect_sizes ();
    DoFTools::make_sparsity_pattern (dof_handler, dsp, constraints, false);

    sparsity_pattern.copy_from(dsp);
    system_matrix.reinit (sparsity_pattern);

    solution.reinit (2);
    solution.block(0).reinit (n_u);
    solution.block(1).reinit (n_p);
    solution.collect_sizes ();

    system_rhs.reinit (2);
    system_rhs.block(0).reinit (n_u);
    system_rhs.block(1).reinit (n_p);
    system_rhs.collect_sizes ();
      


    
  }



  template <int dim>
  void MixedLaplaceProblem<dim>::assemble_system ()
  {
    QGauss<dim>   quadrature_formula(degree+2);
    QGauss<dim-1> face_quadrature_formula(degree+2);

    FEValues<dim> fe_values (fe, quadrature_formula,
                             update_values    | update_gradients |
                             update_quadrature_points  | update_JxW_values);
    FEFaceValues<dim> fe_face_values (fe, face_quadrature_formula,
                                      update_values    | update_normal_vectors |
                                      update_quadrature_points  | update_JxW_values);

    const unsigned int   dofs_per_cell   = fe.dofs_per_cell;
    const unsigned int   n_q_points      = quadrature_formula.size();
    const unsigned int   n_face_q_points = face_quadrature_formula.size();

    FullMatrix<double>   local_matrix (dofs_per_cell, dofs_per_cell);
    Vector<double>       local_rhs (dofs_per_cell);

    std::vector<types::global_dof_index> local_dof_indices (dofs_per_cell);

    const RightHandSide<dim>          right_hand_side;
    const PressureBoundaryValues<dim> pressure_boundary_values;
    const KInverse<dim>               k_inverse;
    const K<dim>               		  k;

    std::vector<double> rhs_values (n_q_points);
    std::vector<double> boundary_values (n_face_q_points);
    std::vector<Tensor<2,dim> > k_inverse_values (n_q_points);
    std::vector<Tensor<2,dim> > k_values (n_q_points);
      

    const FEValuesExtractors::Vector velocities (0);
    const FEValuesExtractors::Scalar pressure (dim);

    typename DoFHandler<dim>::active_cell_iterator
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();
    for (; cell!=endc; ++cell)
      {
        fe_values.reinit (cell);
        local_matrix = 0;
        local_rhs = 0;

        right_hand_side.value_list (fe_values.get_quadrature_points(),
                                    rhs_values);
        k_inverse.value_list (fe_values.get_quadrature_points(),
                              k_inverse_values);
        k.value_list (fe_values.get_quadrature_points(),
                                      k_values);

        for (unsigned int q=0; q<n_q_points; ++q)
          for (unsigned int i=0; i<dofs_per_cell; ++i)
            {
              const Tensor<1,dim> phi_i_u     = fe_values[velocities].value (i, q);
              const double        div_phi_i_u = fe_values[velocities].divergence (i, q);
              const double        phi_i_p     = fe_values[pressure].value (i, q);

              for (unsigned int j=0; j<dofs_per_cell; ++j)
                {
                  const Tensor<1,dim> phi_j_u     = fe_values[velocities].value (j, q);
                  const double        div_phi_j_u = fe_values[velocities].divergence (j, q);
                  const double        phi_j_p     = fe_values[pressure].value (j, q);

                    local_matrix(i,j) += ( 1./data::lambda * phi_i_u * k_inverse_values[q] * phi_j_u
                                        - div_phi_i_u * phi_j_p
                                        - phi_i_p * div_phi_j_u)
                                       * fe_values.JxW(q);
                }

              local_rhs(i) += phi_i_p *
                              rhs_values[q] *
                              fe_values.JxW(q);
              // BE CAREFUL HERE ONCE K is not constant or 1 anymore
            }


        for (unsigned int face_no=0;
             face_no<GeometryInfo<dim>::faces_per_cell;
             ++face_no)
          if (cell->face(face_no)->at_boundary()
                  &&
                  (cell->face(face_no)->boundary_id() == 1))
            {
              fe_face_values.reinit (cell, face_no);
                // DIRICHLET CONDITION FOR TOP

              pressure_boundary_values
              .value_list (fe_face_values.get_quadrature_points(),
                           boundary_values);

              for (unsigned int q=0; q<n_face_q_points; ++q)
                for (unsigned int i=0; i<dofs_per_cell; ++i)
                  local_rhs(i) += -( fe_face_values[velocities].value (i, q) *
                                    fe_face_values.normal_vector(q) *
                                    boundary_values[q] *
                                    fe_face_values.JxW(q));
            }


        cell->get_dof_indices (local_dof_indices);
        for (unsigned int i=0; i<dofs_per_cell; ++i)
          for (unsigned int j=0; j<dofs_per_cell; ++j)
            system_matrix.add (local_dof_indices[i],
                               local_dof_indices[j],
                               local_matrix(i,j));
        for (unsigned int i=0; i<dofs_per_cell; ++i)
          system_rhs(local_dof_indices[i]) += local_rhs(i);
      }
    
    std::map<types::global_dof_index, double> boundary_values_flux; 
    {
            types::global_dof_index n_dofs = dof_handler.n_dofs();
            std::vector<bool> componentVector(dim + 1, true); // condition is on pressue
            // setting flux value for the sides at 0 ON THE PRESSURE
            componentVector[dim] = false;
            std::vector<bool> selected_dofs(n_dofs);
            std::set< types::boundary_id > boundary_ids;
            boundary_ids.insert(0);
        
            DoFTools::extract_boundary_dofs(dof_handler, ComponentMask(componentVector),
                    selected_dofs, boundary_ids);

            for (types::global_dof_index i = 0; i < n_dofs; i++) {
                if (selected_dofs[i]) boundary_values_flux[i] = 0.0; // Side boudaries have flux 0 on pressure
            }

    }

      std::map<types::global_dof_index, double> boundary_values_flux2;
      {
          types::global_dof_index n_dofs2 = dof_handler.n_dofs();
          std::vector<bool> componentVector2(dim + 1, false); // condition is on pressue
          componentVector2[dim+1] = true;
          // SO THE CONDITION IS ON PRESSURE.
          // IF WE WANT TO IMPOSE IT ON FLUX, THEN NEED
          // std::vector<bool> componentVector2(dim + 1, TRUE);
          // componentVector2[dim+1] = FALSE; THEN APPROPRIATE BOUNDARY VAUES ADJUSTED
          std::vector<bool> selected_dofs2(n_dofs2);
          std::set< types::boundary_id > boundary_ids2;
          boundary_ids2.insert(2);
          
          DoFTools::extract_boundary_dofs(dof_handler, ComponentMask(componentVector2),
                                          selected_dofs2, boundary_ids2);
          
          for (types::global_dof_index i = 0; i < n_dofs2; i++) {
              if (selected_dofs2[i]) boundary_values_flux2[i] = -data::rho_f; // bottom boundary with ID 1 has -rho_f flux on pressure
          }
          
      }

      
      
      
    MatrixTools::apply_boundary_values(boundary_values_flux,
            system_matrix, solution, system_rhs);
      
      
      MatrixTools::apply_boundary_values(boundary_values_flux2,
                                         system_matrix, solution, system_rhs);
    
  }





  template <int dim>
  void MixedLaplaceProblem<dim>::solve ()
  {

      SparseDirectUMFPACK  A_direct;
      A_direct.initialize(system_matrix);
      A_direct.vmult (solution, system_rhs);
      
      

  }
    
    template <int dim>
    void MixedLaplaceProblem<dim>::calculate_vf ()
    {


        
    }


  template <int dim>
  void MixedLaplaceProblem<dim>::compute_errors () const
  {
    const ComponentSelectFunction<dim>
    pressure_mask (dim, dim+1);
    const ComponentSelectFunction<dim>
    velocity_mask(std::make_pair(0, dim), dim+1);

    ExactSolution<dim> exact_solution;
    Vector<double> cellwise_errors (triangulation.n_active_cells());

    QTrapez<1>     q_trapez;
    QIterated<dim> quadrature (q_trapez, degree+2);

    VectorTools::integrate_difference (dof_handler, solution, exact_solution,
                                       cellwise_errors, quadrature,
                                       VectorTools::L2_norm,
                                       &pressure_mask);
    const double p_l2_error = cellwise_errors.l2_norm();

    VectorTools::integrate_difference (dof_handler, solution, exact_solution,
                                       cellwise_errors, quadrature,
                                       VectorTools::L2_norm,
                                       &velocity_mask);
    const double u_l2_error = cellwise_errors.l2_norm();

    std::cout << "Errors: ||e_p||_L2 = " << p_l2_error
              << ",   ||e_u||_L2 = " << u_l2_error
              << std::endl;
  }



  template <int dim>
  void MixedLaplaceProblem<dim>::output_results () const
  {
      
      std::vector<std::string> solution_names;
    switch (dim)
      {
      case 2:
        solution_names.push_back ("u");
        solution_names.push_back ("v");
        solution_names.push_back ("p");
        break;

      case 3:
        solution_names.push_back ("u");
        solution_names.push_back ("v");
        solution_names.push_back ("w");
        solution_names.push_back ("p");
        break;

      default:
        Assert (false, ExcNotImplemented());
      }


    DataOut<dim> data_out;

    data_out.attach_dof_handler (dof_handler);
    data_out.add_data_vector (solution, solution_names);


    data_out.build_patches ();

    std::ofstream output ("solution.vtk");
    data_out.write_vtk (output);
  }




  template <int dim>
  void MixedLaplaceProblem<dim>::run ()
  {
    make_grid_and_dofs();
    assemble_system ();
    solve ();
    compute_errors ();
    output_results ();
     // calculate_vf();
    
  }
}



int main ()
{
  try
    {
      using namespace dealii;
      using namespace Step20;

      MixedLaplaceProblem<data::dimension> mixed_laplace_problem(data::problem_degree);
      mixed_laplace_problem.run ();
    }
  catch (std::exception &exc)
    {
      std::cerr << std::endl << std::endl
                << "----------------------------------------------------"
                << std::endl;
      std::cerr << "Exception on processing: " << std::endl
                << exc.what() << std::endl
                << "Aborting!" << std::endl
                << "----------------------------------------------------"
                << std::endl;

      return 1;
    }
  catch (...)
    {
      std::cerr << std::endl << std::endl
                << "----------------------------------------------------"
                << std::endl;
      std::cerr << "Unknown exception!" << std::endl
                << "Aborting!" << std::endl
                << "----------------------------------------------------"
                << std::endl;
      return 1;
    }

  return 0;
}
