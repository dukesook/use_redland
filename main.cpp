#include <cstdlib> // For realpath()
#include <iostream>
#include <redland.h>

void init(librdf_world **world, librdf_storage **storage, librdf_model **model, librdf_parser **parser) {
  *world = librdf_new_world();
  librdf_world_open(*world);
  *storage = librdf_new_storage(*world, "memory", NULL, NULL);
  *model = librdf_new_model(*world, *storage, NULL);
  *parser = librdf_new_parser(*world, "turtle", NULL, NULL);
  std::cout << "Redland initialized successfully." << std::endl;
}

void parse_file(librdf_world *world, librdf_parser *parser, librdf_model *model, const char *filename) {
  std::cout << "Parsing file..." << std::endl;
  std::string file_uri = "file://" + std::string(realpath(filename, NULL));
  librdf_uri *uri = librdf_new_uri(world, (const unsigned char *)file_uri.c_str());

  if (librdf_parser_parse_into_model(parser, uri, uri, model)) {
    std::cerr << "Failed to parse the RDF file: " << filename << std::endl;
    exit(1);
  }
  std::cout << "Successfully loaded RDF file: " << filename << std::endl;

  librdf_free_uri(uri);
}

librdf_query_results *execute_sparql(librdf_world *world, librdf_model *model, const char *sparql) {
  // Create and execute the query
  librdf_query *query = librdf_new_query(world, "sparql", NULL, (const unsigned char *)sparql, NULL);
  if (!query) {
    std::cerr << "Failed to create SPARQL query." << std::endl;
    exit(1);
  }

  librdf_query_results *results = librdf_query_execute(query, model);
  if (!results || librdf_query_results_finished(results)) {
    std::cerr << "No results returned from the SPARQL query." << std::endl;
    exit(1);
  }

  std::cout << "Query executed successfully.\n";
  librdf_free_query(query);
  return results;
}

void print_results(librdf_query_results *results) {
  std::cout << "Results:\n";
  while (!librdf_query_results_finished(results)) {
    librdf_node *s = librdf_query_results_get_binding_value(results, 0);
    librdf_node *p = librdf_query_results_get_binding_value(results, 1);
    librdf_node *o = librdf_query_results_get_binding_value(results, 2);

    std::cout << "- Triple: ";
    std::cout << (s ? (const char *)librdf_node_to_string(s) : "(null)") << " ";
    std::cout << (p ? (const char *)librdf_node_to_string(p) : "(null)") << " ";
    std::cout << (o ? (const char *)librdf_node_to_string(o) : "(null)") << std::endl;

    librdf_query_results_next(results);
  }
}

int main() {
  // Initialize the Redland world
  librdf_world *world;
  librdf_storage *storage;
  librdf_model *model;
  librdf_parser *parser;

  // Initialize Redland
  init(&world, &storage, &model, &parser);

  // Load RDF File
  // const char *filename = "small.ttl";
  const char *filename = "sample_rdf_glas.ttl";
  parse_file(world, parser, model, filename);

  // Execute SPARQL
  const char *sparql_query = "SELECT ?s ?p ?o WHERE { ?s ?p ?o } LIMIT 10";
  librdf_query_results *results = execute_sparql(world, model, sparql_query);

  // Print Results
  print_results(results);

  // Cleanup
  librdf_free_query_results(results);
  librdf_free_parser(parser);
  librdf_free_model(model);
  librdf_free_storage(storage);
  librdf_free_world(world);

  return 0;
}
