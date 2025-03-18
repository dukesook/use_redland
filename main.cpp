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

std::string create_ephemeris_query(std::string image_id) {
  // clang-format off
    std::string ephemeris_query = 
        "PREFIX obi: <http://purl.obolibrary.org/obo/OBI_> "
        "PREFIX cco: <http://www.ontologyrepository.com/CommonCoreOntologies/> "
        "PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> "
        "PREFIX imh: <http://ontology.mil/foundry/IMH_> "
        "SELECT ?x ?y ?z ?t WHERE { "
        "  ?ephemeris cco:is_about <" + image_id + ">. "
        "  ?ephemeris rdf:type imh:0001646. "
        "  ?ephemeris imh:0001670 ?coordinate. "
        "  ?coordinate imh:0001430 ?x. "
        "  ?coordinate imh:0001474 ?y. "
        "  ?coordinate imh:0001447 ?z. "
        "  ?coordinate imh:0001161 ?tinst. "
        "  ?tinst obi:0002135 ?t. "
        "}";
      return ephemeris_query;
  // clang-format on
}

void example_query1(librdf_world *world, librdf_model *model) {

  // Create Query
  const char *sparql_query = "SELECT ?s ?p ?o WHERE { ?s ?p ?o } LIMIT 10";

  // Execute Query
  librdf_query_results *results = execute_sparql(world, model, sparql_query);

  // Print Results
  std::cout << "\nTriples:\n";
  while (!librdf_query_results_finished(results)) {
    librdf_node *s = librdf_query_results_get_binding_value(results, 0);
    librdf_node *p = librdf_query_results_get_binding_value(results, 1);
    librdf_node *o = librdf_query_results_get_binding_value(results, 2);

    std::cout << (s ? (const char *)librdf_node_to_string(s) : "(null)") << " ";
    std::cout << (p ? (const char *)librdf_node_to_string(p) : "(null)") << " ";
    std::cout << (o ? (const char *)librdf_node_to_string(o) : "(null)") << std::endl;

    librdf_query_results_next(results);
  }

  // Cleanup
  librdf_free_query_results(results);
}

void example_query2(librdf_world *world, librdf_model *model) {
  // Create Query
  const char *image_id = "http://fresh.com/19"; // unique identifier to specify a particular image
  std::string sparql_query = create_ephemeris_query(image_id);

  // Execute Query
  librdf_query_results *results = execute_sparql(world, model, sparql_query.c_str());

  // Print Results
  std::cout << "\nEphemeris:\n";
  while (!librdf_query_results_finished(results)) {
    librdf_node *x = librdf_query_results_get_binding_value(results, 0);
    librdf_node *y = librdf_query_results_get_binding_value(results, 1);
    librdf_node *z = librdf_query_results_get_binding_value(results, 2);
    librdf_node *t = librdf_query_results_get_binding_value(results, 3);

    std::cout << "  x: " << (x ? (const char *)librdf_node_get_literal_value(x) : "(null)") << " ";
    std::cout << "  y: " << (y ? (const char *)librdf_node_get_literal_value(y) : "(null)") << " ";
    std::cout << "  z: " << (z ? (const char *)librdf_node_get_literal_value(z) : "(null)") << " ";
    std::cout << "  t: " << (t ? (const char *)librdf_node_get_literal_value(t) : "(null)") << std::endl;

    librdf_query_results_next(results);
  }

  librdf_free_query_results(results);
}

void cleanup(librdf_world *world, librdf_storage *storage, librdf_model *model, librdf_parser *parser) {
  librdf_free_parser(parser);
  librdf_free_model(model);
  librdf_free_storage(storage);
  librdf_free_world(world);
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
  const char *filename = "sample_rdf_glas.ttl";
  parse_file(world, parser, model, filename);

  // Execute SPARQL #1
  example_query1(world, model);

  // Execute SPARQL #2
  example_query2(world, model);

  // Cleanup
  cleanup(world, storage, model, parser);
  return 0;
}
