<?php

/**
 * @file
 * Create demo landing pages.
 */

use Drupal\menu_link_content\Entity\MenuLinkContent;
use Drupal\node\Entity\Node;
use Drupal\paragraphs\Entity\Paragraph;
use Drupal\vicgovau_demo\VicgovauDemoHelper;
use Drupal\vicgovau_demo\VicgovauDemoRepository;

/**
 * Implements hook_vicgovau_demo_create_ENTITY_TYPE_BUNDLE_weight().
 */
function vicgovau_demo_create_node_landing_page_weight() {
  return 100;
}

/**
 * Implements hook_vicgovau_demo_create_ENTITY_TYPE_BUNDLE().
 */
function vicgovau_demo_create_node_landing_page() {
  $nodes = [];
  $repository = VicgovauDemoRepository::getInstance();
  $site_menus = $repository->getDemoEntities('site_menu');

  for ($i = 1; $i <= 10; $i++) {
    $uid = VicgovauDemoHelper::randomUid();

    $site_sections = VicgovauDemoHelper::randomSiteSections();
    $primary_site_id = VicgovauDemoHelper::randomSite($site_sections);

    $status = VicgovauDemoHelper::randomBool();
    $node_data = [
      'type' => 'landing_page',
      'title' => VicgovauDemoHelper::randomSentence(),
      'uid' => $uid,
      'status' => $status,
      'field_landing_page_intro_text' => [
        'value' => VicgovauDemoHelper::randomPlainParagraph(),
      ],
      'body' => [
        'value' => VicgovauDemoHelper::randomRichTextWithMedia(),
        'format' => 'rich_text',
      ],
      'field_node_site' => VicgovauDemoHelper::getFieldValueForSiteSections($site_sections),
      'field_node_primary_site' => [
        ['target_id' => $primary_site_id],
      ],
      'field_topic' => [
        ['target_id' => VicgovauDemoHelper::randomTopic()],
      ],
      'field_featured_image' => [
        ['target_id' => VicgovauDemoHelper::randomImage()],
      ],
    ];

    // Add Key Journeys.
    $key_journeys_data = [
      'type' => 'key_journeys',
      'field_paragraph_title' => 'Want to know more about...',
      'field_paragraph_cta' => [VicgovauDemoHelper::randomCtaLinkFieldValue()],
    ];
    for ($j = 1; $j <= 2; $j++) {
      $key_journeys_data['field_paragraph_links'][] = VicgovauDemoHelper::randomLinkFieldValue();
    }
    $key_journeys = Paragraph::create($key_journeys_data);
    $key_journeys->save();
    $repository->trackEntity($key_journeys);
    $node_data['field_landing_page_key_journeys'] = [
      [
        'target_id' => $key_journeys->id(),
        'target_revision_id' => $key_journeys->getRevisionId(),
      ],
    ];

    // Add Related links.
    for ($j = 1; $j <= mt_rand(1, 5); $j++) {
      $related_link_data = [
        'type' => 'related_links',
        'field_paragraph_summary' => ['value' => VicgovauDemoHelper::randomPlainParagraph()],
        'field_paragraph_link' => [VicgovauDemoHelper::randomLinkFieldValue()],
      ];
      $related_link = Paragraph::create($related_link_data);
      $related_link->save();
      $repository->trackEntity($related_link);
      $node_data['field_related_links'][] = [
        'target_id' => $related_link->id(),
        'target_revision_id' => $related_link->getRevisionId(),
      ];
    }

    // Add What's Next.
    for ($j = 1; $j <= mt_rand(1, 3); $j++) {
      $whats_next_data = [
        'type' => 'links',
        'field_paragraph_summary' => [['value' => VicgovauDemoHelper::randomPlainParagraph()]],
        'field_paragraph_link' => [VicgovauDemoHelper::randomLinkFieldValue()],
      ];
      $whats_next = Paragraph::create($whats_next_data);
      $whats_next->save();
      $repository->trackEntity($whats_next);
      $node_data['field_whats_next'][] = [
        'target_id' => $whats_next->id(),
        'target_revision_id' => $whats_next->getRevisionId(),
      ];
    }

    // Add components.
    $components = VicgovauDemoHelper::randomLandingPageComponents();
    foreach ($components as $component) {
      /** @var \Drupal\paragraphs\Entity\Paragraph $component */
      $node_data['field_landing_page_component'][] = [
        'target_id' => $component->id(),
        'target_revision_id' => $component->getRevisionId(),
      ];
    }

    $node = Node::create($node_data);

    $booleanFields = [
      'field_show_related_content',
      'field_show_social_sharing',
      'field_show_whats_next',
      'field_show_content_rating',
    ];
    foreach ($booleanFields as $booleanField) {
      if ($node->hasField($booleanField)) {
        $node->set($booleanField, VicgovauDemoHelper::randomBool());
      }
    }

    if ($node->hasField('moderation_state')) {
      if ($status) {
        $node->set('moderation_state', 'published');
      }
    }

    try {
      $node->save();
      $nodes[] = $node;

      // Create menu links.
      foreach ($site_sections as $site_id) {
        foreach (['main', 'footer'] as $menu_prefix) {
          if (isset($site_menus[$menu_prefix . ':site:' . $site_id])) {
            /** @var \Drupal\system\Entity\Menu $menu */
            $menu = reset($site_menus[$menu_prefix . ':site:' . $site_id]);
            $menu_item = MenuLinkContent::create([
              'title' => $node->getTitle(),
              'link' => 'entity:node/' . $node->id(),
              'menu_name' => $menu->id(),
              'expanded' => TRUE,
            ]);
            $menu_item->save();
            $repository->trackEntity($menu_item);
          }
        }
      }
    }
    catch (Exception $exception) {
      watchdog_exception('vicgovau_demo', $exception);
    }
  }

  return $nodes;
}