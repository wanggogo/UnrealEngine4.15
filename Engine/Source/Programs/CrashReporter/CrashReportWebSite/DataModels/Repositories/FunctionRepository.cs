﻿using System;
using System.Collections.Generic;
using System.Data.Entity;
using System.Linq;
using System.Linq.Expressions;

namespace Tools.CrashReporter.CrashReportWebSite.DataModels.Repositories
{
    /// <summary>
    /// Repository for function calls
    /// </summary>
    public class FunctionRepository : IFunctionRepository
    {
        private CrashReportEntities _entityContext;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="entityContext">A reference to the *single* instance of the data context.</param>
        public FunctionRepository(CrashReportEntities entityContext)
        {
            _entityContext = entityContext;
        }

        /// <summary>
        /// Return a queryable string for query construction.
        /// 
        /// NOTE - This is bad. Replace this method with proper expression tree construction and strictly return
        /// enumerated lists. All data handling should happen within the repository. 
        /// </summary>
        /// <returns></returns>
        public IQueryable<FunctionCall> ListAll()
        {
            return _entityContext.FunctionCalls;
        }

        /// <summary>
        /// Count the number of objects that satisfy the filter
        /// </summary>
        /// <param name="filter"></param>
        /// <returns></returns>
        public int Count(Expression<Func<FunctionCall, bool>> filter)
        {
            return _entityContext.FunctionCalls.Count(filter);
        }

        /// <summary>
        /// Get a filtered list of Buggs from data storage
        /// Calling this method returns the data directly. It will execute the data retrieval - in this case an sql transaction.
        /// </summary>
        /// <param name="filter">A linq expression used to filter the Bugg table</param>
        /// <returns>Returns a fully filtered enumerated list object of Bugges.</returns>
        public IEnumerable<FunctionCall> Get(Expression<Func<FunctionCall, bool>> filter)
        {
            return _entityContext.FunctionCalls.Where(filter).ToList();
        }

        /// <summary>
        /// Get a filtered, ordered list of Bugg
        /// </summary>
        /// <param name="filter">An expression used to filter the Buggs</param>
        /// <param name="orderBy">An function delegate userd to order the results from the Bugg table</param>
        /// <returns></returns>
        public IEnumerable<FunctionCall> Get(Expression<Func<FunctionCall, bool>> filter,
            Func<IQueryable<FunctionCall>, IOrderedQueryable<FunctionCall>> orderBy)
        {
            return orderBy(_entityContext.FunctionCalls.Where(filter)).ToList();
        }

        /// <summary>
        /// Return an ordered list of FunctionCalls with data pre-loading
        /// </summary>
        /// <param name="filter">A linq expression used to filter the Bugg table</param>
        /// <param name="orderBy">A linq expression used to order the results from the Bugg table</param>
        /// <param name="includeProperties">A linq expression indicating properties to dynamically load.</param>
        /// <returns></returns>
        public IEnumerable<FunctionCall> Get(Expression<Func<FunctionCall, bool>> filter,
            Func<IQueryable<FunctionCall>, IOrderedQueryable<FunctionCall>> orderBy,
            params Expression<Func<FunctionCall, object>>[] includeProperties)
        {
            var query = _entityContext.FunctionCalls.Where(filter);
            foreach (var includeProperty in includeProperties)
            {
                query.Include(includeProperty);
            }

            return orderBy == null ? query.ToList() : orderBy(query).ToList();
        }

        /// <summary>
        /// Get a Function call from it's id
        /// </summary>
        /// <param name="id">The id of the Bugg to retrieve</param>
        /// <returns>Bugg data model</returns>
        public FunctionCall GetById(int id)
        {
            return _entityContext.FunctionCalls.FirstOrDefault(data => data.Id == id);
        }

        /// <summary>
        /// Check if there are any function calls matching a specific filter.
        /// </summary>
        /// <param name="filter"></param>
        /// <returns></returns>
        public bool Any(Expression<Func<FunctionCall, bool>> filter)
        {
            return _entityContext.FunctionCalls.Any(filter);
        }

        /// <summary>
        /// Get the first function calls matching a specific filter.
        /// </summary>
        /// <param name="filter"></param>
        /// <returns></returns>
        public FunctionCall First(Expression<Func<FunctionCall, bool>> filter)
        {
            return _entityContext.FunctionCalls.FirstOrDefault(filter);
        }

        /// <summary>
        /// Get the first function calls matching a specific filter.
        /// </summary>
        /// <param name="filter"></param>
        /// <returns></returns>
        public int FirstId(Expression<Func<FunctionCall, bool>> filter)
        {
            return _entityContext.FunctionCalls.Where(filter).Select(data => data.Id).First();
        }

        /// <summary>
        /// Add a new Bugg to the data store
        /// </summary>
        /// <param name="entity"></param>
        public void Save(FunctionCall entity)
        {
            _entityContext.FunctionCalls.Add(entity);
        }

        /// <summary>
        /// Remove a Bugg from the data store
        /// </summary>
        /// <param name="entity"></param>
        public void Delete(FunctionCall entity)
        {
            _entityContext.FunctionCalls.Remove(entity);
        }

        /// <summary>
        /// Update an existing Bugg
        /// </summary>
        /// <param name="entity"></param>
        public void Update(FunctionCall entity)
        {
            var set = _entityContext.Set<FunctionCall>();
            var entry = set.Local.SingleOrDefault(f => f.Id == entity.Id);

            if (entry != null)
            {
                var attachedFeature = _entityContext.Entry(entry);
                attachedFeature.CurrentValues.SetValues(entity);
                attachedFeature.State = EntityState.Modified;
            }
            else
            {
                _entityContext.FunctionCalls.Attach(entity);
                _entityContext.Entry(entity).State = EntityState.Modified;
            }
        }
    }
}