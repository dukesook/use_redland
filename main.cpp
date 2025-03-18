#include <cstdlib> // For realpath()
#include <iostream>
#include <redland.h>

void init(librdf_world **world, librdf_storage **storage, librdf_model **model, librdf_parser **parser) {
  *world = librdf_new_world();
  librdf_world_open(*world);

  *storage = librdf_new_storage(*world, "memory", NULL, NULL);

  *model = librdf_new_model(*world, *storage, NULL);

  *parser = librdf_new_parser(*world, "turtle", NULL, NULL);
}

void parse_file(librdf_world *world, librdf_parser *parser, librdf_model *model, const char *filename) {
  std::string file_uri = "file://" + std::string(realpath(filename, NULL));
  librdf_uri *uri = librdf_new_uri(world, (const unsigned char *)file_uri.c_str());

  if (librdf_parser_parse_into_model(parser, uri, uri, model)) {
    std::cerr << "Failed to parse the RDF file: " << filename << std::endl;
    exit(1);
  }
  std::cout << "Successfully loaded RDF file: " << filename << std::endl;

  librdf_free_uri(uri);
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
  const char *filename = "small.ttl";
  parse_file(world, parser, model, filename);

  // Define SPARQL query: Get first 10 triples
  const char *sparql_query = "SELECT ?s ?p ?o WHERE { ?s ?p ?o } LIMIT 10";

  // Create and execute the query
  librdf_query *query = librdf_new_query(world, "sparql", NULL, (const unsigned char *)sparql_query, NULL);
  if (!query) {
    std::cerr << "Failed to create SPARQL query." << std::endl;
    return 1;
  }

  librdf_query_results *results = librdf_query_execute(query, model);
  if (!results || librdf_query_results_finished(results)) {
    std::cerr << "No results returned from the SPARQL query." << std::endl;
    return 1;
  }

  std::cout << "Query executed successfully. Results:\n";

  // Iterate through results
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

  // Cleanup
  librdf_free_query_results(results);
  librdf_free_query(query);
  librdf_free_parser(parser);
  librdf_free_model(model);
  librdf_free_storage(storage);
  librdf_free_world(world);

  return 0;
}
